


typedef struct {
    /** Number of rows of the CSV matrix. */
    long nrows;
    /** Number of columns of the CSV matrix. */
    long ncols;
    /** Header of the CSV. */
    char **header;
    /** CSV matrix of strings. Each string has 512 characters. */
    char ***data;
} iftCSV;

%extend iftCSV {

	~iftCSV() {
		iftCSV* ptr = ($self);
		iftDestroyCSV(&ptr);
	}
};

%newobject iftCreateCSV;
%feature("autodoc", "2");
iftCSV *iftCreateCSV(long nrows, long ncols);

%newobject iftReadCSV;
%feature("autodoc", "2");
iftCSV *iftReadCSV(const char *csv_pathname, const char separator);

%feature("autodoc", "2");
void iftWriteCSV(   iftCSV *csv, const char *filename, const char separator);

%feature("autodoc", "2");
void iftPrintCSV(   iftCSV *csv);

%feature("autodoc", "2");
iftCSV *iftMergeCSVs(   iftCSV *csv1,    iftCSV *csv2);

%feature("autodoc", "2");
iftCSV *iftCopyCSV(   iftCSV *csv);

%feature("autodoc", "2");
void iftSetCSVHeader(iftCSV *csv, const char *header);

