#ifndef IO_XML_H
#define IO_XML_H

#include "Base/ueShapes.h"

/**
 *	@addtogroup io
 *	@{
 */

/**
 *	@struct ioXmlDoc
 *	@brief Xml document handle.
 */
struct ioXmlDoc;
/**
 *	@struct ioXmlNode
 *	@brief Xml nide handle.
 */
struct ioXmlNode;
/**
 *	@struct ioXmlAttr
 *	@brief Xml attribute handle.
 */
struct ioXmlAttr;

// Xml document

//! Creates Xml document
ioXmlDoc*		ioXmlDoc_Create(ueAllocator* allocator, const char* version = "1.0", const char* encoding = "utf-8");
//! Saves Xml document to a file
ueBool			ioXmlDoc_Save(ioXmlDoc* doc, const char* path);
//! Loads Xml document from a file
ioXmlDoc*		ioXmlDoc_Load(ueAllocator* allocator, const char* fileName);
//! Loads Xml document from string; note: string will be modified
ioXmlDoc*		ioXmlDoc_LoadFromString(ueAllocator* allocator, char* text);
//! Destroys Xml document
void			ioXmlDoc_Destroy(ioXmlDoc* doc);
//! Gets first child node with matching name of an Xml document
ioXmlNode*		ioXmlDoc_GetFirstNode(ioXmlDoc* doc, const char* name = NULL);
//! Adds child node of given name to an Xml document
ioXmlNode*		ioXmlDoc_AddNode(ioXmlDoc* doc, const char* name);

// Xml node

//! Gets next Xml node with matching name
ioXmlNode*		ioXmlNode_GetNext(ioXmlNode* node, const char* name = NULL);
//! Gets Xml node name
const char*		ioXmlNode_GetName(const ioXmlNode* node);
//! Gets Xml node value
const char*		ioXmlNode_GetValue(const ioXmlNode* node);
//! Gets first Xml child node with matching name
ioXmlNode*		ioXmlNode_GetFirstNode(ioXmlNode* node, const char* name = NULL);
//! Calculates number of Xml child nodes with matching name
u32				ioXmlNode_CalcNumNodes(ioXmlNode* node, const char* name = NULL);
//! Gets first Xml attribute node with matching name
ioXmlAttr*		ioXmlNode_GetFirstAttr(ioXmlNode* node, const char* name = NULL);
//! Gets value of the first Xml attribute with matching name; returns NULL if not found
const char*		ioXmlNode_GetAttrValue(ioXmlNode* node, const char* name);
//! Gets value of the first boolean Xml attribute with matching name; returns UE_TRUE if found and successfully converted to boolean, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueBool(ioXmlNode* node, const char* name, ueBool& result);
//! Gets value of the first float Xml attribute with matching name; returns UE_TRUE if found and successfully converted to float, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueF32(ioXmlNode* node, const char* name, f32& result);
//! Gets value of the first integer Xml attribute with matching name; returns UE_TRUE if found and successfully converted to integer, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueS32(ioXmlNode* node, const char* name, s32& result);
//! Gets value of the first unsigned integer Xml attribute with matching name; returns UE_TRUE if found and successfully converted to unsigned integer, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueU32(ioXmlNode* node, const char* name, u32& result);
//! Gets value of the first vector-2 Xml attribute with matching name; returns UE_TRUE if found and successfully converted to vector-2, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueVec2(ioXmlNode* node, const char* name, ueVec2& result);
//! Gets value of the first vector-4 Xml attribute with matching name; returns UE_TRUE if found and successfully converted to vector-4, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueVec4(ioXmlNode* node, const char* name, ueVec4& result);
//! Gets value of the first rectangle Xml attribute with matching name; returns UE_TRUE if found and successfully converted to rectangle, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueRect(ioXmlNode* node, const char* name, ueRect& result);
//! Gets value of the first color-32 Xml attribute with matching name; returns UE_TRUE if found and successfully converted to color-32, UE_FALSE otherwise
ueBool			ioXmlNode_GetAttrValueColor32(ioXmlNode* node, const char* name, ueColor32& result);

//! Gets value of the first Xml attribute with matching name; returns default value if not found
const char*		ioXmlNode_GetAttrValue(ioXmlNode* node, const char* name, const char* defaultValue);
//! Gets value of the first boolean Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueBool(ioXmlNode* node, const char* name, ueBool& result, const ueBool defaultValue);
//! Gets value of the first float Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueF32(ioXmlNode* node, const char* name, f32& result, const f32 defaultValue);
//! Gets value of the first integer Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueS32(ioXmlNode* node, const char* name, s32& result, const s32 defaultValue);
//! Gets value of the first unsigned integer Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueU32(ioXmlNode* node, const char* name, u32& result, const u32 defaultValue);
//! Gets value of the first vector-2 Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueVec2(ioXmlNode* node, const char* name, ueVec2& result, const ueVec2& defaultValue);
//! Gets value of the first vector-3 Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueVec3(ioXmlNode* node, const char* name, ueVec3& result, const ueVec3& defaultValue);
//! Gets value of the first vector-4 Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueVec4(ioXmlNode* node, const char* name, ueVec4& result, const ueVec4& defaultValue);
//! Gets value of the first quaternion Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueQuat(ioXmlNode* node, const char* name, ueQuat& result, const ueQuat& defaultValue);
//! Gets value of the first rectangle Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueRect(ioXmlNode* node, const char* name, ueRect& result, const ueRect& defaultValue);
//! Gets value of the first color-32 Xml attribute with matching name; returns default value if not found
void			ioXmlNode_GetAttrValueColor32(ioXmlNode* node, const char* name, ueColor32& result, const ueColor32 defaultValue);

//! Adds child Xml node to given node
ioXmlNode*		ioXmlNode_AddNode(ioXmlNode* node, const char* name);
//! Adds child comment Xml node to given node
ioXmlNode*		ioXmlNode_AddCommentNode(ioXmlNode* _node, const char* comment);
//! Adds child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttr(ioXmlNode* node, const char* name, const char* value);
//! Adds boolean child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrBool(ioXmlNode* node, const char* name, ueBool value);
//! Adds 32-bit integer child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrS32(ioXmlNode* node, const char* name, s32 value);
//! Adds 32-bit unsigned integer child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrU32(ioXmlNode* node, const char* name, u32 value);
//! Adds float child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrF32(ioXmlNode* node, const char* name, f32 value);
//! Adds vector-2 child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrVec2(ioXmlNode* node, const char* name, const ueVec2& value);
//! Adds vector-4 child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrVec4(ioXmlNode* node, const char* name, const ueVec4& value);
//! Adds rectangle child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrRect(ioXmlNode* node, const char* name, const ueRect& value);
//! Adds color-32 child Xml attribute to node
ioXmlAttr*		ioXmlNode_AddAttrColor32(ioXmlNode* node, const char* name, ueColor32 value);

// Xml attribute

//! Gets next Xml attribute
ioXmlAttr*		ioXmlAttr_GetNext(ioXmlAttr* attr);
//! Gets Xml attribute name
const char*		ioXmlAttr_GetName(const ioXmlAttr* attr);
//! Gets Xml attribute value
const char*		ioXmlAttr_GetValue(const ioXmlAttr* attr);

//! Scoped Xml destruction helper
class ioXmlDocScopedDestructor
{
public:
	//! Assigns Xml document
	UE_INLINE ioXmlDocScopedDestructor(ioXmlDoc* doc) : m_doc(doc) {}
	//! Destroys Xml document
	UE_INLINE ~ioXmlDocScopedDestructor() { if (m_doc) ioXmlDoc_Destroy(m_doc); }
private:
	ioXmlDoc* m_doc; //!< Handled Xml document
};

// @}

#endif // IO_XML_H
