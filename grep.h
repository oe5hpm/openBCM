#define LMAX   260
#define PMAX   200
#define CHAR   1
#define BOL    2
#define EOL    3
#define ANY    4
#define CLASS  5
#define NCLASS 6
#define STAR   7
#define PLUS   8
#define RANGE  9
#define REGEX  10
#define QUICK  11
#define CASESENSE 12
#define TOUPPER   13
#define ENDPAT    14

/*---------------------------------------------------------------------------*/
class CRegEx
{
  public:
    int regex_compile (char *source);
    int grepfile (FILE *fp);
    int regex_match (char *line);
    int regex_match (char *pattern, char *line);
    CRegEx();
    char m_pattern[PMAX];
    bitfeld m_options;

  private:
  char *pmatch (char *begin, char *line, char *pattern);
};
