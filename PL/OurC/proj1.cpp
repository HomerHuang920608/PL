# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define NOT !
# define AND &&
# define OR ||
# define EQ ==

enum TokenType { IDENTIFIER = 1, CONSTANT = 2, SPECIAL = 3 } ;
typedef char * CharPtr ;    // the strings of all tokens are to be stored in the heap

struct Column {
  int column ;
  Column * next ;
} ; // Column

typedef Column * ColumnPtr ;

struct Line {
  int line ;
  ColumnPtr firstAppearAt ;
  ColumnPtr lastAppearAt ;
  Line * next ;
} ; // Line

typedef Line * LinePtr ;

struct Token {
  CharPtr tokenStr ;
  TokenType type ;
  LinePtr firstAppearOn ;
  LinePtr lastAppearOn ;
  Token * next ;
} ; // Token

typedef Token * TokenPtr ;

static int uTestNum = -1 ;

int gLine = 1 ;    // the line-no of of the char that is yet to be read in
int gColumn = 1 ;  // the column-no of of the char that is yet to be read in

char gNextChar = '\0' ;    // the char we just read in
int gNextCharLine = -1 ;   // line-no of gNextChar (the char we just read in)
int gNextCharColumn = -1 ; // column-no of gNextChar (the char we just read in)

TokenPtr gFront = NULL ;   // !!! the stored token list is empty at the start

typedef char Str40[100] ;

bool gNext = false ;

void GetNextChar( char & ch, int & line, int & column ) {

  scanf( "%c", &ch ) ;
  if ( gNext ) { 
    line++ ;
    column = 0 ;
    gNext = false ;
  } // if

  if ( line == -1 ) {
    line = gLine ;
    column = gColumn ;
  } // if
  else if ( ch == '\n' ) {
    gNext = true ;
    column++ ;
  } // else if
  else column++ ;

} // GetNextChar()

bool IsDigit( char ch ) {

  if ( ch == '0' )  return true ;
  else if ( ch == '1' )  return true ;
  else if ( ch == '2' )  return true ;
  else if ( ch == '3' )  return true ;
  else if ( ch == '4' )  return true ;
  else if ( ch == '5' )  return true ;
  else if ( ch == '6' )  return true ;
  else if ( ch == '7' )  return true ;
  else if ( ch == '8' )  return true ;
  else if ( ch == '9' )  return true ;
  return false ;

} // IsDigit()

bool IsLetter( char ch ) {

  if ( ch >= 65 && ch <= 90 )  return true ;
  else if ( ch >= 97 && ch <= 122 )  return true ;
  return false ;

} // IsLetter()

bool IsWhiteSpace( char ch ) {

  if ( ch == '\n' )  return true ;
  else if ( ch == ' ' )  return true ;
  else if ( ch == '\t' )  return true ;
  return false ;

} // IsWhiteSpace()

bool IsSpecial( char ch ) {

  if ( ch == '^' )  return true ;
  else if ( ch == ',' )  return true ;
  else if ( ch == '(' )  return true ;
  else if ( ch == ')' )  return true ;
  else if ( ch == '[' )  return true ;
  else if ( ch == ']' )  return true ;
  else if ( ch == '{' )  return true ;
  else if ( ch == '}' )  return true ;
  else if ( ch == '!' )  return true ;
  else if ( ch == ':' )  return true ;
  else if ( ch == ';' )  return true ;
  else if ( ch == '#' )  return true ;
  else if ( ch == '?' )  return true ;
  else if ( ch == '+' )  return true ;
  else if ( ch == '-' )  return true ;
  else if ( ch == '*' )  return true ;
  else if ( ch == '/' )  return true ;
  else if ( ch == '>' )  return true ;
  else if ( ch == '<' )  return true ;
  else if ( ch == '=' )  return true ;
  else if ( ch == '%' )  return true ;
  else if ( ch == '&' )  return true ;
  else if ( ch == '|' )  return true ;
  return false ;

} // IsSpecial()

void SkipWhiteSpaces() {
  
  while ( gNextChar == ' ' || gNextChar == '\n' || gNextChar == '\t' ) {
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // while

} // SkipWhiteSpaces()

void SkipLineComment() {

  while ( gNextChar != '\n' ) {
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // while  

  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;

} // SkipLineComment()

void SkipMultiLineComment() {
  char before = '\0' ;
  while ( true ) {
    before = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
    if ( before == '*' && gNextChar == '/' ) {
      GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
      return ;
    } // if
  } // while

} // SkipMultiLineComment()

CharPtr GetID() {

  Str40 id = "\0" ;
  int i = 0 ;
  while ( IsLetter( gNextChar ) || IsDigit( gNextChar ) || gNextChar == '_' ) {
    id[i] = gNextChar ;
    i++ ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // while

  CharPtr idPtr = new Str40 ;
  strcpy( idPtr, id ) ;
  return idPtr ;

} // GetID() 

CharPtr GetNum() {

  Str40 num = "\0" ;
  int i = 0 ;
  while ( IsDigit( gNextChar ) || gNextChar == '.' ) {
    num[i] = gNextChar ;
    i++ ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // while

  CharPtr numPtr = new Str40 ;
  strcpy( numPtr, num ) ;
  return numPtr ;
  
} // GetNum()

CharPtr GetCharConst() {

  Str40 charconst = "\0" ;
  int i = 1 ;
  char before = '\0' ;

  charconst[0] = '\'' ;
  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  while ( gNextChar != '\'' || before == '\\' ) {
    charconst[i] = gNextChar ;
    i++ ;
    before = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // while

  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  charconst[i] = '\'' ;
  CharPtr charconstPtr = new Str40 ;
  strcpy( charconstPtr, charconst ) ;
  return charconstPtr ;

} // GetCharConst()

CharPtr GetStringConst() {

  Str40 strconst = "\0" ;
  int i = 1 ;
  char before = '\0' ;

  strconst[0] = '\"' ;
  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  while ( gNextChar != '\"' || before == '\\' ) {
    strconst[i] = gNextChar ;
    i++ ;
    before = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // while

  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  strconst[i] = '\"' ;
  CharPtr strconstPtr = new Str40 ;
  strcpy( strconstPtr, strconst ) ;
  return strconstPtr ;

} // GetStringConst()

CharPtr GetSpecial() {

  Str40 special = "\0" ;
  special[0] = gNextChar ;
  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;

  if ( special[0] == '+' ) {
    if ( gNextChar == '=' || gNextChar == '+' ) {
      special[1] = gNextChar ;
      GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
    } // if
  } // if
  else if ( special[0] == '-' ) {
    if ( gNextChar == '=' || gNextChar == '>' || gNextChar == '-' ) {
      special[1] = gNextChar ;
      GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
    } // if
  } // else if
  else if ( special[0] == '*' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '/' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '>' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '<' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '!' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '%' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '&' && gNextChar == '&' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '|' && gNextChar == '|' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '>' && gNextChar == '>' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '<' && gNextChar == '<' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if
  else if ( special[0] == '=' && gNextChar == '=' ) {
    special[1] = gNextChar ;
    GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
  } // else if

  CharPtr specialPtr = new Str40 ;
  strcpy( specialPtr, special ) ;
  return specialPtr ;

} // GetSpecial()

CharPtr GetToken( TokenType & tokenType, int & firstCharLine, int & firstCharColumn ) {
  
  bool slashIsDivide = false ;
  int lineOfDivide = -1 ;
  int columnOfDivide = -1 ;

  while ( IsWhiteSpace( gNextChar ) || ( gNextChar == '/' && ! slashIsDivide ) ) {
    if ( IsWhiteSpace( gNextChar ) ) SkipWhiteSpaces() ;
    else if ( gNextChar == '/' ) {
      lineOfDivide = gNextCharLine ;
      columnOfDivide = gNextCharColumn ;

      GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
      if ( gNextChar == '/' )  SkipLineComment() ;
      else if ( gNextChar == '*' )  SkipMultiLineComment() ;
      else  slashIsDivide = true ;
    } // else if
  } // while

  firstCharLine = gNextCharLine ;
  firstCharColumn = gNextCharColumn ;
  CharPtr tokenStr = NULL ;

  if ( slashIsDivide ) {
    if ( gNextChar == '=' ) {
      tokenStr = new Str40 ;
      tokenType = SPECIAL ;
      GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;
      strcpy( tokenStr, "/=" ) ;
      firstCharColumn -= 1 ;
    } // if
    else {
      tokenStr = new Str40 ;
      tokenType = SPECIAL ;
      strcpy( tokenStr, "/" ) ;
      firstCharLine = lineOfDivide ;
      firstCharColumn = columnOfDivide ;
    } // else 
  } // if
  else if ( IsLetter( gNextChar ) || gNextChar == '_' ) {
    tokenStr = GetID( ) ;
    tokenType = IDENTIFIER ;
  } // if
  else if ( IsDigit( gNextChar ) || gNextChar == '.' ) {
    tokenStr = GetNum( ) ;
    tokenType = CONSTANT ;
  } // else if
  else if ( gNextChar == '\'' ) {
    tokenStr = GetCharConst( ) ;
    tokenType = CONSTANT ;
  } // else if
  else if ( gNextChar == '\"' ) {
    tokenStr = GetStringConst( ) ;
    tokenType = CONSTANT ;
  } // else if
  else if ( IsSpecial( gNextChar ) ) {
    tokenStr = GetSpecial( ) ;
    tokenType = SPECIAL ;
  } // else if

  return tokenStr ;

} // GetToken()

void InsertToken( TokenPtr & head, CharPtr tokenStr, 
                  TokenType tokenType, int tokenLine, int tokenColumn ) {

  Token * tokenInsert = new Token ;
  tokenInsert->tokenStr = tokenStr ;
  tokenInsert->type = tokenType ;
  tokenInsert->next = NULL ;

  if ( head == NULL ) {
    head = tokenInsert ;
  } // if
  else InsertToken( head->next, tokenStr, tokenType, tokenLine, tokenColumn ) ;

} // InsertToken()

void PrintTokenInfo( TokenPtr head ) {

  TokenPtr temp = head ;

  while ( temp != NULL ) {

    if ( strcmp( temp->tokenStr, ";" ) == 0 ) printf( "\n" ) ; 
    else printf( "%s", temp->tokenStr ) ;

    temp = temp->next ;

  } // while

} // PrintTokenInfo()

void PrintAll() {

    printf( "Program starts...\n" ) ;

    printf( "Program exists..." ) ;

} // PrintAll

int main() {
  char ch = '\0' ;

  CharPtr tokenStr = NULL ;
  int tokenLine = 0, tokenColumn = 0 ;
  TokenType tokenType ;

  TokenPtr listHead = NULL ;   // !!! empty token list at the start

  scanf( "%d%c", &uTestNum, &ch ) ;

  GetNextChar( gNextChar, gNextCharLine, gNextCharColumn ) ;

  do {

    tokenStr = GetToken( tokenType, tokenLine, tokenColumn ) ;

    if ( strcmp( tokenStr, "quit" ) != 0 )
      InsertToken( listHead, tokenStr, tokenType, tokenLine, tokenColumn ) ;

  } while ( strcmp( tokenStr, "quit" ) != 0 ) ;
  
  PrintTokenInfo( listHead ) ;
  
  // ----------Scanner----------

  return 0 ;

} // main()
