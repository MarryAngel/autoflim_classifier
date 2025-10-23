


typedef struct {
    /** Array of chars (int8), the less datatype size (1 byte). */
    char *val;
    /** Array size: Number char elements (bytes) from bitmap (char array). */
    int nbytes;
    /** Number of available bits (bitmap elements) to be set: nbytes * 8 */
    int n;
} iftBMap;

%extend iftBMap {

	~iftBMap() {
		iftBMap* ptr = ($self);
		iftDestroyBMap(&ptr);
	}
};

%newobject iftCreateBMap;
%feature("autodoc", "2");
iftBMap *iftCreateBMap(int n);

%newobject iftReadBMap;
%feature("autodoc", "2");
iftBMap *iftReadBMap(const char *path);

%feature("autodoc", "2");
void iftWriteBMap(   iftBMap *bmap, const char *path);

%feature("autodoc", "2");
void iftFillBMap(iftBMap *bmap, int value);

%newobject iftCopyBMap;
%feature("autodoc", "2");
iftBMap *iftCopyBMap(   iftBMap *src);

%feature("autodoc", "2");
static inline void iftBMapSet0(iftBMap *bmap, int b) ;

%feature("autodoc", "2");
static inline void iftBMapSet1(iftBMap *bmap, int b) ;

%feature("autodoc", "2");
static inline bool iftBMapValue(   iftBMap *bmap, int b) ;

%feature("autodoc", "2");
static inline void iftBMapToggle(iftBMap *bmap, int b) ;

