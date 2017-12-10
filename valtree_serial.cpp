/* Copyright 2013 by Pavel Chistyakov
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "crt.h"
#include "io_plugin.h"
#include "valtree.h"

static void write_object(io::writer& file, valtree* object)
{
	switch(object->type)
	{
	case valtree::Structure:
		file.open(object->name);
		for(auto p = object->value; p; p = p->next)
			write_object(file, p);
		file.close(object->name);
		break;
	case valtree::Text:
		file.set(object->name, (const char*)object->value);
		break;
	case valtree::Number:
		file.set(object->name, (int)object->value);
		break;
	}
}

void valtree::write(const char* url)
{
	auto pp = io::plugin::find(szext(url));
	if(!pp)
		return;
	io::file file(url, StreamWrite | StreamText);
	if(!file)
		return;
	io::writer* pw = pp->write(file);
	if(pw)
		write_object(*pw, this);
}

bool valtree::read(const char* url)
{
	struct valtree_reader : public io::reader
	{
		valtree* root;

		void open(io::reader::node& e)
		{
			if(e.parent)
			{
				auto pp = (valtree*)e.parent->params[0];
				e.params[0] = (int)&pp->set(e.name);
			}
			else
			{
				root->name = szdup(e.name);
				root->set(valtree::Structure);
				e.params[0] = (int)root;
			}
		}

		void set(io::reader::node& e, int value)
		{
			auto pp = (valtree*)e.parent->params[0];
			pp->set(e.name, value);
		}

		void set(io::reader::node& e, const char* value)
		{
			auto pp = (valtree*)e.parent->params[0];
			pp->set(e.name, value);
		}

	} reader;
	reader.root = this;
	return io::read(url, reader);
}