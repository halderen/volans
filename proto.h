#ifndef PROTO_H
#define PROTO_H

/*
 * Definitions relating to an iterator.  An iterator is a object handle that
 * allows you to loop over the elements contained in some abstract data
 * structure.  The properties are dictated by the data structure, e.g. if the
 * data structure guarantees a certain order, then the elements are returned
 * in order, and whether the elements may be modified is also governed by
 * the data structure.  An iterator is obtained from the data structure
 * related functions.  The iterator functions only hide the implementation
 * details of the data structure, so that you do not faced whether to follow
 * a next pointer or other method to traverse the abstract data structure.
 * 
 * An iterator is a cursor into a set of items.  Initial, the cursor is
 * placed on the first item.  The entire set of items must be iterated over,
 * or the end() call must be used to terminate the iteration of elements,
 * however it should be assumed performance is directly proportionally with
 * the number of items in the set, NOT the actual number of items iterated
 * over.  Additionally obtaining an iterator should be assumed to have a
 * significant performance impact.  With both assumptions in mind, you
 * should get the right type of iteration in place, which retrieved indeed
 * the set of items needed, rather than just one every time or all of the
 * items in the data structure.
 * 
 * Two typical usaged could be:
 *     struct mystruct* item;
 *     iterator iter = getiterator(...);
 *     if(iterate(&iter, &item)) {
 *         printf("%s",item->myname);
 *         while(advance(&iter, &item)) {
 *             printf(",%s", item->myname);
 *             if(need_bail_out()) {
 *                 end(&iter);
 *                 break;
 *             }
 *         }
 *         printf("\n");
 *     } else
 *         printf("There are no items\n");
 * Or:
 *     for(iter=getiterator(...); iterate(&iter, &item); advance(&iter,NULL))
 *         printf("%s\n",item->myname);
 * 
 * The iterate() call returns whether the cursor is not yet beyond the end of
 * the set of items in the iteration.  If the second argument is not the NULL
 * pointer, the current item is returned in it.
 * 
 * The advance() call advances the cursor to the next element in the list.
 * If the cursor advances past the last item, the end() call is implicitly
 * executed.  If the second argument is not NULL, the item pointed to by the
 * cursor after advancing is returned in it.
 * 
 * The end() call terminates the iteration prematurely and releases any
 * memory or locks implied by the iterator.  If will always return
 * successful.
 */
typedef struct names_iterator_struct* names_iterator;

int names_iterate(names_iterator*iter, void* item);
int names_advance(names_iterator*iter, void* item);
int names_end(names_iterator*iter);

names_iterator names_iterator_create(size_t size);
void names_iterator_add(names_iterator i, void* ptr);
void names_iterator_addall(names_iterator iter, int count, void* base, size_t memsize, ssize_t offset);
names_iterator names_iterator_array(int count, void* base, size_t memsize, size_t offset);
names_iterator names_iterator_array2(int count, void* base, size_t memsize);

struct marshall_struct;
typedef struct marshall_struct* marshall_handle;

enum marshall_method { marshall_INPUT, marshall_OUTPUT, marshall_APPEND, marshall_PRINT };
marshall_handle marshallcreate(enum marshall_method method, ...);
void marshallclose(marshall_handle h);
int marshallself(marshall_handle h, void* member);
int marshallbyte(marshall_handle h, void* member);
int marshallinteger(marshall_handle h, void* member);
int marshallstring(marshall_handle h, void* member);
int marshallstringarray(marshall_handle h, void* member);
int marshalling(marshall_handle h, char* name, void* members, int *membercount, size_t membersize, int (*memberfunction)(marshall_handle,void*));

extern int* marshall_OPTIONAL;

/* A dictionary is an abstract data structure capable of storing key
 * value pairs, where each value is again a dictionary.
 * A (sub)dictionary can also have a name.
 * 
 * The purpose is for the moment as placeholder and to be replaced with
 * the domain structure, containing the denial, rrset, etcetera structures.
 */

typedef struct dictionary_struct* dictionary;
typedef struct names_index_struct* names_index_type;
typedef struct names_table_struct* names_table_type;
typedef struct names_view_struct* names_view_type;

void composestring(char* dst, const char* src, ...);
int composestring2(char** ptr, const char* src, ...);
int composestringf(char** ptr, const char* fmt, ...);
int getset(dictionary d, const char* name, const char** get, const char** set);

struct item {
    char* data;
    char* info;
};

dictionary names_recordcreate(char**name);
void annotate(dictionary, const char* apex);
void names_recorddestroy(dictionary);
void names_recordsetmarker(dictionary dict);
int names_recordhasmarker(dictionary dict);
dictionary names_recordcopy(dictionary);
void dispose(dictionary);
const char* names_recordgetid(dictionary dict, const char* name);
int names_recordcompare_namerevision(dictionary a, dictionary b);
int names_recordhasdata(dictionary, char* name, char* data, char* info);
void names_recordadddata(dictionary, char* name, char* data, char* info);
void names_recorddeldata(dictionary, char* name, char* data);
void names_recorddelall(dictionary, char* name);
names_iterator names_recordalltypes(dictionary);
names_iterator names_recordallvalues(dictionary, char*name);
int names_recordhasvalidupto(dictionary);
int names_recordgetvalidupto(dictionary);
void names_recordsetvalidupto(dictionary, int value);
int names_recordhasvalidfrom(dictionary);
int names_recordgetvalidfrom(dictionary);
void names_recordsetvalidfrom(dictionary, int value);
int names_recordhasexpiry(dictionary);
int names_recordgetexpiry(dictionary);
void names_recordsetexpiry(dictionary, int value);
void names_recordsetsignature(dictionary record, char*name, char* signature);
int names_recordmarshall(dictionary*, marshall_handle);
void names_recordindexfunction(const char* keyname, int (**acceptfunction)(dictionary newitem, dictionary currentitem, int* cmp), int (**comparefunction)(const void *, const void *));

struct dual {
    dictionary src;
    dictionary dst;
};

int names_indexcreate(names_index_type*, const char* keyname);
dictionary names_indexlookup(names_index_type, dictionary);
dictionary names_indexlookupkey(names_index_type, const char* keyvalue);
int names_indexremove(names_index_type, dictionary);
int names_indexremovekey(names_index_type,const char* keyvalue);
int names_indexinsert(names_index_type, dictionary);
void names_indexdestroy(names_index_type, void (*userfunc)(void* arg, void* key, void* val), void* userarg);
int names_indexaccept(names_index_type, dictionary);
names_iterator names_indexiterator(names_index_type);
names_iterator names_indexrange(names_index_type,char* selection,...);

names_iterator noexpiry(names_view_type);
names_iterator neighbors(names_view_type);
names_iterator expiring(names_view_type);

/* Table structures are used internally by views to record changes made in
 * the view.  A table is a set of changes, also dubbed a changelog.
 * The table* functions are not to be used outside of the scope of the
 * names_ module.
 */

names_table_type names_tablecreate(void);
void names_tabledispose(names_table_type table, void (*userfunc)(void* arg, void* key, void* val), void* userarg);
void* names_tableget(names_table_type table, const char* name);
int names_tabledel(names_table_type table, char* name);
void** names_tableput(names_table_type table, const char* name);
void names_tableconcat(names_table_type* list, names_table_type item);
names_iterator names_tableitems(names_table_type table);

/* The changelog_ functions are also not to be used directly, they
 * extend the table functionality in combination with the views.
 */

typedef struct names_commitlog_struct* names_commitlog_type;

void names_commitlogdestroy(names_table_type changelog);
void names_commitlogdestroyall(names_commitlog_type views, marshall_handle* store);
int names_commitlogpoppush(names_commitlog_type, int viewid, names_table_type* previous, names_table_type* mychangelog);
int names_commitlogsubscribe(names_view_type view, names_commitlog_type*);
void names_commitlogpersistincr(names_commitlog_type, names_table_type changelog);
void names_commitlogpersistappend(names_commitlog_type, void (*persistfn)(names_table_type, marshall_handle), marshall_handle store);
int names_commitlogpersistfull(names_commitlog_type, void (*persistfn)(names_table_type, marshall_handle), int viewid, marshall_handle store, marshall_handle* oldstore);

void names_own(names_view_type view, dictionary* record);
void names_amend(names_view_type view, dictionary record);
void* names_place(names_view_type store, const char* name);
void* names_take(names_view_type view, int index, const char* name);
void names_remove(names_view_type view, dictionary record);
names_view_type names_viewcreate(names_view_type base, const char* name, const char** keynames);
void names_viewdestroy(names_view_type view);
names_iterator names_viewiterator(names_view_type view, int index);
names_iterator names_viewiterate(names_view_type view, char* name, ...);
int names_viewcommit(names_view_type view);
void names_viewreset(names_view_type view);
int names_viewpersist(names_view_type view, int basefd, char* filename);
int names_viewrestore(names_view_type view, const char* apex, int basefd, const char* filename);

void names_dumprecord(FILE*, dictionary record);
void names_dumpviewinfo(names_view_type view);
void names_dumpviewfull(FILE*, names_view_type view);

struct signconf;
struct signconf* createsignconf(int nkeys);
void locatekeysignconf(struct signconf* signconf, int index, const char* locator);
void destroysignconf(struct signconf* signconf);
void setupsignconf(struct signconf* signconf);
void teardownsignconf(struct signconf* signconf);
void signrecord(struct signconf* signconf, dictionary record);
void signrecord2(struct signconf* signconf, dictionary record, char* apex);
void sign(names_view_type view);
void prepare(names_view_type view, int newserial);
void writezone(names_view_type view, const char* filename, const char* apex, int* defaultttl);
enum operation_enum { PLAIN, DELTAMINUS, DELTAPLUS };
int readzone(names_view_type view, enum operation_enum operation, const char* filename, char** apexptr, int* defaultttlptr);
void rr2data(ldns_rr* rr, char** recorddataptr, char** recordinfoptr);

struct names_struct {
    names_view_type baseview;
    names_view_type inputview;
    names_view_type prepareview;
    names_view_type signview;
    names_view_type outputview;
    int basefd;
    char* apex;
    char* source;
    char* persist;
};

int names_docreate(struct names_struct** zoneptr, const char* apex, const char* persist, const char* input);
void names_dodestroy(struct names_struct* names);
void names_docycle(struct names_struct* names, int* serial, const char* filename);
void names_dopersist(struct names_struct* names);

#endif
