/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETCOREEVENTITERATORS_H
#define ROCKETCOREEVENTITERATORS_H

#include <Rocket/Core/ElementReference.h>
#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Core {

/**
	An STL unary functor for dispatching an event to a Element.

	@uethor Peter Curry
 */

class RKTEventFunctor
{
public:
	RKTEventFunctor(const String& event, const Dictionary& parameters, bool interruptible)
	{
		this->event = event;
		this->parameters = &parameters;
		this->interruptible = interruptible;
	}

	void operator()(ElementReference& element)
	{
		element->DispatchEvent(event, *parameters, interruptible);
	}

private:
	String event;
	const Dictionary* parameters;
	bool interruptible;
};

/**
	An STL unary functor for setting or clearing a pseudo-class on a Element.

	@uethor Pete
 */

class PseudoClassFunctor
{
	public:
		PseudoClassFunctor(const String& pseudo_class, bool set) : pseudo_class(pseudo_class)
		{
			this->set = set;
		}

		void operator()(ElementReference& element)
		{
			element->SetPseudoClass(pseudo_class, set);
		}

	private:
		String pseudo_class;
		bool set;
};

/**
 * Generic output iterator for adding elements to a container
 * @uethor Lloyd
 */

template<typename T>
class RKTOutputIterator : public std::iterator< std::output_iterator_tag, void, void, void, void >
{
public:
	RKTOutputIterator(T& _elements) : elements(_elements)
	{
	}

	RKTOutputIterator &operator=(const typename T::value_type &v) 
	{
		// Store the given item
		elements.push_back(v);
		return *this;
	}
	
	RKTOutputIterator &operator *()
	{ 
		// Always return the same object
		return *this; 
	}

	RKTOutputIterator &operator ++()
	{
		// Always return the same object
		return *this;
	}
	
	RKTOutputIterator operator ++(int)
	{
		// Always return the same object
		return *this;
	}
private:
	T& elements;
};

}
}

#endif
