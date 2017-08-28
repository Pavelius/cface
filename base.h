#pragma once
#define FO(c,f) (const int)&((c*)0)->f

// Standart tags used to work with bases
// Used interval from 0xDF00 to 0xE000
enum base_tags
{
	// Common properties
	Name = 0xDF00, Description, Identifier, Key, Parent, Value, Type, Filter, Path, Flags,
	Width, Height, Count,
	Modified,
	LineNumber, ColumnNumber, LineHeight,
	FirstBaseRequisit = Name, LastBaseRequisit = LineHeight,
	// Controls and types
	Text, Number, Date,
	Boolean, // Type that have view in 'yes/no' style. Like checkbox.
	Structure,
	Array,
	Table,
	Image, // View has image of image sprite.
	Flag, // Bits flags data. Parent is base for shift. Formula in bits is 1<<(This-Parent-1). So if we have 'RecordState, Inherted, Definded' then 'Inherted' was 1 bit, 'Defined' 2 bit.
	Reference, // Any reference, not only n specific parent hierarhy
	Point, Color,
	FirstBaseType = Text, LastBaseType = Color,
	// Conditions
	Conditions,
	Equal, NotEqual, Lesser, LesserEqual, Greater, GreaterEqual, Between, InList, NotInList, InHierarhy, Like,
	// Common parent
	Commands, Selectors,
	// Table commands
	Add, AddCopy, Change, Delete, MoveUp, MoveDown, MoveLeft, MoveRight, Setting, SortAscending, SortDescending,
	TableExport, TableImport,
	Update, Clear, Visibility, Usability,
	// Application coommands
	Create, Open, Save,
	Cut, Copy, Paste,
	Toolbar, Group,
	Cancel, Continue, OK,
	First, Last, LastPossible,
	FirstBaseTag = Name, LastBaseTag = LastPossible,
};
void					bsadd(int rec, int id, int value);
void					bsadd(int rec, int id, int value, int minimum);
void					bsadd(int rec, int id, int value, int minimum, int maximum);
int						bscreate(int rec, bool test_zero=true);
int						bscompareas(const void* p1, const void* p2);
int						bscompareds(const void* p1, const void* p2);
int						bscomparenm(const void* p1, const void* p2);
void					bsdelete(int rec);
int						bsfind(int rec, int id, int value);
int						bsfind(int rec, int id, int value, int id2, int value2);
int						bsfirstby(int rec, int par, int id);
int						bsget(int rec, int id);
const char*				bsgets(int rec, int id);
const char*				bsgets(int rec, int id, int locale);
bool					bshierarhy(int rec, int par, int id);
int						bsrec(const char* value);
int*					bsselect(int* result, int r1, int r2);
int*					bsselect(int* result, int r1, int r2, int i1, int v1);
int*					bsselect(int* result, const int* source, int i1, int v1);
void					bsset(int rec, int id, int value);
void					bsset(int rec, int id, const char* value);
void					bsset(int rec, int id, const char* value, int locale);
int*					bssort(int* elements);
int						bstag(const char* string);
int*					bsuniq(int* result);