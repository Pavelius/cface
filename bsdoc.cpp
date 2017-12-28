#include "crt.h"
#include "io.h"
#include "bsdoc.h"

static const char* index_header = "<h1>File format description</h1>"
"<p>Each line of description file consist of one entity decriptor.</p>"
"<p>Entity descriptor start line with symbol '#' and then name of entity.</p>"
"<p>Several entities has big text description which used to display tooltips and other text multiline controls. This special text lines can be after entity block and before next entitity.</p>"
"<h2>Entity list</h2>";
static const char* text_description = "Entity description text. Start with new line after entiry.";
static const char* index_header_global = "<h2>Global namespaces</h1>";

static void opentag(io::stream& e, const char* name)
{
	e << "<" << name << ">";
}

static void closetag(io::stream& e, const char* name)
{
	e << "</" << name << ">";
}

static void header(io::stream& e)
{
	opentag(e, "html");
	opentag(e, "body");
	e << "<style type = \"text/css\">";
	e << "	TABLE{";
	e << "    border-collapse: collapse;";
	//e << "    border: 1px solid gray;";
	e << "  }";
	e << "  TD, TH{";
	e << "    padding: 5px;";
	e << "    text-align: left;";
	e << "  }";
	e << "</style>";
}

static void footer(io::stream& e)
{
	closetag(e, "body");
	closetag(e, "html");
}

static int compare_metadata(const void* p1, const void* p2)
{
	return strcmp((*((bsdata**)p1))->id, (*((bsreq**)p2))->id);
}

static void write_ref(io::stream& e, bsdata& m, const char* path = 0)
{
	e << "<a href=\"";
	if(path)
		e << path << "/";
	e << m.id << ".html\">" << m.id << "</a>";
}

static void write_type(io::stream& e, const bsreq* f)
{
	if(f == number_type)
		e << "number";
	else if(f == text_type)
		e << "string";
	else
	{
		auto m = bsdata::find(f);
		if(m)
			write_ref(e, *m);
	}
}

static void attribute(io::stream& e, const char* name, int value)
{
	e << " " << name << "=\"" << value << "\"";
}

static void attribute(io::stream& e, const char* name, const char* value)
{
	e << " " << name << "=\"" << value << "\"";
}

static void opentable(io::stream& e, int columns)
{
	e << "<table";
	if(columns)
		attribute(e, "cols", columns);
	e << ">";
}

//static void write_xsref(io::stream& e, xsref& xr, const xsfield* f)
//{
//	for(unsigned i = 0; i < f->count; i++)
//	{
//		if(f->type == number_type)
//		{
//			if(i > 0)
//				e << ", ";
//			e << xr.get(f->id, i);
//		}
//		else if(f->type == text_type)
//		{
//			if(i > 0)
//				e << ", ";
//			e << "\"" << (const char*)xr.get(f->id, i) << "\"";
//		}
//		else if(f->reference)
//		{
//			auto pid = (const char*)xr.getr(f->id, i).get("id");
//			if(pid)
//			{
//				if(i > 0)
//					e << ", ";
//				e << pid;
//			}
//			else
//				break;
//		}
//		else
//		{
//			auto xp = xr.getr(f->id, i);
//			for(auto pf = xp.fields; *pf; pf++)
//			{
//				if(pf != xp.fields)
//					e << ", ";
//				write_xsref(e, xp, pf);
//			}
//		}
//	}
//}
//
//static void write_xsref(io::stream& e, xsref& xr)
//{
//	auto m = xsbase::find(xr.fields);
//	if(!m)
//		return;
//	e << "#" << m->id;
//	if(xr.fields->find("id"))
//		e << " " << (const char*)xr.get("id");
//	else
//		e << " " << m->indexof(xr.object);
//	for(auto f = xr.fields; *f; f++)
//	{
//		if(strcmp(f->id, "id") == 0)
//			continue;
//		if(strcmp(f->id, "text") == 0)
//			continue;
//		if(!xr.get(f->id))
//			continue;
//		e << " " << f->id << "(";
//		write_xsref(e, xr, f);
//		e << ")";
//	}
//	e << "\n\r";
//}

static void write_examples(io::stream& e, bsdata& m)
{
	if(m.getcount() < 2)
		return;
	opentag(e, "section");
	opentag(e, "h3");
	e << "Examples";
	closetag(e, "h3");
	int start = 0;
	if(m.fields->find("id") == 0)
		start++;
	for(int i = start; i < 4; i++)
	{
		//auto xr = m.getref(i);
		//if(!xr.object)
		//	break;
		opentag(e, "code");
		//write_xsref(e, xr);
		closetag(e, "code");
		opentag(e, "br");
	}
	closetag(e, "section");
}

static void write_header(io::stream& e, bsdata& m, bsdoc& doc)
{
	char temp[256];
	opentag(e, "section");
	opentag(e, "h1");
	zcpy(temp, m.id, sizeof(temp) - 1);
	szupper(temp, 1);
	e << temp;
	closetag(e, "h1");
	e << "This is entity represent " << m.id << ".";
	auto p = doc.comments->find(m.id);
	if(p && p->text && p->text[0])
		e << " " << p->text;
	closetag(e, "section");
}

static void write_field(io::stream& e, const bsreq* f)
{
	if(strcmp(f->id, "id") == 0)
		return;
	if(strcmp(f->id, "text") == 0)
		return;
	e << " ";
	e << f->id << "(<i>";
	write_type(e, f->type);
	if(f->count > 1)
		e << ", ...";
	e << "</i>)";
}

static void write_syntaxis(io::stream& e, bsdata& m)
{
	if(!m.data)
		return;
	opentag(e, "section");
	opentag(e, "h3");
	e << "Syntaxis";
	closetag(e, "h3");
	opentag(e, "code");
	e << "<b>#" << m.id << "</b>";
	if(m.fields->find("id"))
		e << " id";
	else
		e << " index";
	for(auto f = m.fields; *f; f++)
		write_field(e, f);
	if(m.fields->find("text"))
		e << "<br>" << "<i>text</i>";
	closetag(e, "code");
	closetag(e, "section");
}

static void write_metaobject(bsdata& m, bsdoc& doc)
{
	char temp[260];
	io::file e(szurl(temp, "help/metadata", m.id, "html"), StreamWrite | StreamText);
	if(!e)
		return;
	auto pp = doc.comments->find(m.id);
	header(e);
	write_header(e, m, doc);
	write_syntaxis(e, m);
	opentag(e, "section");
	opentag(e, "h3");
	e << "Fields description";
	closetag(e, "h3");
	//opentag(e, "table");
	opentable(e, 3);
	opentag(e, "tr");
	opentag(e, "th");
	e << "Name";
	closetag(e, "th");
	opentag(e, "th");
	e << "Type";
	closetag(e, "th");
	opentag(e, "th");
	e << "Description";
	closetag(e, "th");
	closetag(e, "tr");
	for(auto f = m.fields; *f; f++)
	{
		opentag(e, "tr");
		opentag(e, "td");
		e << f->id;
		closetag(e, "td");
		opentag(e, "td");
		write_type(e, f->type);
		closetag(e, "td");
		opentag(e, "td");
		if(strcmp(f->id, "id") == 0)
			e << "Use this value to make reference on this element in another objects.";
		else if(strcmp(f->id, "name") == 0)
			e << "Name used to display element.";
		else if(strcmp(f->id, "text") == 0)
			e << text_description;
		else
		{
			stringtree* p = 0;
			if(pp)
				p = pp->elements->find(f->id);
			if(!p)
				p = doc.comments->find(f->id);
			if(p && p->text && p->text[0])
				e << p->text << " ";
			if(f->count > 1)
				e << "Maximum " << f->count << " elements. ";
		}
		closetag(e, "td");
		closetag(e, "tr");
	}
	closetag(e, "table");
	closetag(e, "section");
	write_examples(e, m);
	footer(e);
}

static void write_metadata(io::stream& e, bsdoc& doc)
{
	bsdata* meta[512];
	int count = 0;
	for(auto m = bsdata::first; m; m = m->next)
		meta[count++] = m;
	qsort(meta, count, sizeof(meta[0]), compare_metadata);
	opentag(e, "table");
	for(auto i = 0; i < count; i++)
	{
		write_metaobject(*meta[i], doc);
		if(meta[i]->data)
		{
			opentag(e, "tr");
			opentag(e, "td");
			write_ref(e, *meta[i], "metadata");
			closetag(e, "td");
			closetag(e, "tr");
		}
	}
	closetag(e, "table");
}

void bsdoc::generate()
{
	char temp[260];
	io::file e(szurl(temp, "help", "index", "html"), StreamWrite | StreamText);
	if(!e)
		return;
	header(e);
	e << index_header;
	write_metadata(e, *this);
	e << index_header_global;
	footer(e);
}

bsdoc::bsdoc() : comments(0)
{
	p = porigin = 0;
	buffer[0] = 0;
	memset(operation, 0, sizeof(operation));
}