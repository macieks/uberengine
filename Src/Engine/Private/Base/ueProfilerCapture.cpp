#include "Base/ueBase.h"

#if defined(UE_ENABLE_PROFILER)

#include "Base/ueProfilerCapture.h"
#include "Base/ueProfiler_Private.h"
#include "Base/Containers/ueHashMap.h"

#define UE_PROFILER_MAX_THREADS 256
#define UE_PROFILER_MAX_STACK 64

void ueProfiler_AnalyzeData(const ueProfilerData* profilerData, u32& numThreads, ueThreadId* threadIds, ueHashMap<ueThreadId, u8>& threadIndexById)
{
	numThreads = 0;
	threadIndexById.Clear();

	const u8* data = (u8*) profilerData->m_buffer;
	const u8* dataEnd = (const u8*) profilerData->m_buffer + profilerData->m_bufferSize;
	while (data < dataEnd)
	{
		const ueProfilerOperation* _op = (const ueProfilerOperation*) data;

		// Move to next operation

		switch (_op->m_type)
		{
			case ueProfilerOperation::Type_Push:
				data += sizeof(ueProfilerOperation_Push);
				break;
			case ueProfilerOperation::Type_PushDynamic:
			{
				const ueProfilerOperation_PushDynamic* op = (const ueProfilerOperation_PushDynamic*) _op;
				data += op->GetSize();
				break;
			}
			case ueProfilerOperation::Type_Pop:
				data += sizeof(ueProfilerOperation_Pop);
				break;
		}

		// Record thread usage

		if (!threadIndexById.Find(_op->m_threadId))
		{
			threadIndexById.Insert(_op->m_threadId, numThreads);
			threadIds[numThreads] = _op->m_threadId;
			numThreads++;
		}
	}
}

void ueProfiler_ReverseChildren(ueProfilerNode* parent)
{
	if (!parent->m_firstChild || !parent->m_firstChild->m_nextSibling)
		return;

	ueProfilerNode* prev = NULL;
	ueProfilerNode* curr = parent->m_firstChild;
	while (curr)
	{
		ueProfilerNode* next = curr->m_nextSibling;
		curr->m_nextSibling = prev;
		prev = curr;
		curr = next;
	}
	parent->m_firstChild = prev;
}

ueProfilerCapture* ueProfiler_BuildCapture(void* dst, u32 dstSize, const ueProfilerData* profilerData)
{
	if (profilerData->m_bufferSize == 0)
		return NULL;

	// Analyze the data

	u32 numThreads = 0;
	ueThreadId threadIds[UE_PROFILER_MAX_THREADS];

	ueHashMap<ueThreadId, u8> threadIdLookupTable;
	threadIdLookupTable.Init(g_profilerMgrData->m_allocator, UE_PROFILER_MAX_THREADS);

	ueProfiler_AnalyzeData(profilerData, numThreads, threadIds, threadIdLookupTable);

	u8* memory = (u8*) dst;
	const u8* memoryEnd = memory + dstSize;

	// Build capture

	ueProfilerCapture* capture = (ueProfilerCapture*) memory;
	memory += sizeof(ueProfilerCapture);
	capture->m_startTime = 0;
	capture->m_numTrees = numThreads;
	capture->m_trees = (ueProfilerTree*) memory;
	memory += sizeof(ueProfilerTree) * numThreads;

	for (u32 i = 0; i < numThreads; i++)
	{
		capture->m_trees[i].m_isValid = UE_TRUE;
		capture->m_trees[i].m_root = NULL;
		capture->m_trees[i].m_threadId = threadIds[i];
	}

	// Process all operations

	ueProfilerNode* treeStacks[UE_PROFILER_MAX_THREADS][UE_PROFILER_MAX_STACK]; memset(treeStacks, 0, sizeof(treeStacks));
	u32 treeStackSizes[UE_PROFILER_MAX_THREADS]; memset(treeStackSizes, 0, sizeof(treeStackSizes));
	
	const u8* data = (u8*) profilerData->m_buffer;
	const u8* dataEnd = (const u8*) profilerData->m_buffer + profilerData->m_bufferSize;
	while (data < dataEnd)
	{
		const ueProfilerOperation* _op = (const ueProfilerOperation*) data;

		const u32 treeIndex = *threadIdLookupTable.Find(_op->m_threadId);
		ueProfilerTree* tree = &capture->m_trees[treeIndex];

		switch (_op->m_type)
		{
			case ueProfilerOperation::Type_Push:
			{
				// Get operation

				const ueProfilerOperation_Push* op = (const ueProfilerOperation_Push*) _op;
				data += sizeof(ueProfilerOperation_Push);

				// Detect capture start time

				if (capture->m_startTime == 0)
					capture->m_startTime = op->m_ticks;

				// Create node

				ueProfilerNode* node = (ueProfilerNode*) memory;
				if (memory + sizeof(ueProfilerNode) > memoryEnd)
					break;
				memory += sizeof(ueProfilerNode);
				node->m_enterNanoSecs = (u32) ueClock_TimeToNanoSecs(op->m_ticks);
				node->m_firstChild = NULL;
				node->m_nextSibling = NULL;
				node->m_name = ueProfiler_GetScopeName(op->m_id);

				// Update stack

				u32& stackSize = treeStackSizes[treeIndex];
				ueProfilerNode*& newStackTop = treeStacks[treeIndex][stackSize];

				if (stackSize > 0)
				{
					ueProfilerNode* stackTop = treeStacks[treeIndex][stackSize - 1];
					node->m_nextSibling = stackTop->m_firstChild;
					stackTop->m_firstChild = node;
				}
				else
					capture->m_trees[treeIndex].m_root = node;

				newStackTop = node;
				stackSize++;
				break;
			}

			case ueProfilerOperation::Type_PushDynamic:
			{
				// Get operation

				const ueProfilerOperation_PushDynamic* op = (const ueProfilerOperation_PushDynamic*) _op;
				data += op->GetSize();

				// Detect capture start time

				if (capture->m_startTime == 0)
					capture->m_startTime = op->m_ticks;

				// Create node

				ueProfilerNode* node = (ueProfilerNode*) memory;
				if (memory + op->GetSize() > memoryEnd)
					break;
				memory += sizeof(ueProfilerNode);
				node->m_enterNanoSecs = (u32) ueClock_TimeToNanoSecs(op->m_ticks);
				node->m_firstChild = NULL;
				node->m_nextSibling = NULL;

				ueMemCpy(memory, op->GetName(), op->m_nameSize);
				node->m_name = (char*) memory;
				memory += op->m_nameSize;

				// Update stack

				u32& stackSize = treeStackSizes[treeIndex];
				ueProfilerNode*& newStackTop = treeStacks[treeIndex][stackSize];

				if (stackSize > 0)
				{
					ueProfilerNode* stackTop = treeStacks[treeIndex][stackSize - 1];
					node->m_nextSibling = stackTop->m_firstChild;
					stackTop->m_firstChild = node;
				}
				else
					capture->m_trees[treeIndex].m_root = node;

				newStackTop = node;
				stackSize++;
				break;
			}

			case ueProfilerOperation::Type_Pop:
			{
				// Get operation

				const ueProfilerOperation_Pop* op = (const ueProfilerOperation_Pop*) _op;
				data += sizeof(ueProfilerOperation_Pop);

				// Get stack

				u32& stackSize = treeStackSizes[treeIndex];
				ueProfilerNode*& stackTop = treeStacks[treeIndex][stackSize - 1];

				// Update inclusive and exclusive node times

				stackTop->m_inclNanoSecs = (u32) ueClock_TimeToNanoSecs(op->m_ticks) - stackTop->m_enterNanoSecs;
				stackTop->m_exclNanoSecs = stackTop->m_inclNanoSecs;

				ueProfilerNode* child = stackTop->m_firstChild;
				while (child)
				{
					stackTop->m_exclNanoSecs -= child->m_inclNanoSecs;
					child = child->m_nextSibling;
				}

				// Reverse children nodes

				ueProfiler_ReverseChildren(stackTop);

				// Update stack

				stackTop = NULL;
				stackSize--;
				break;
			}
		}
	}
	
	return capture;
}

#endif // defined(UE_ENABLE_PROFILER)