# include <stdlib.h>
# include <string>
# include <iostream>
# include <vector>
# include <iomanip>
# include <map>

using namespace std ;

enum TokenType {

    LEFT_PAREN,   // (
    RIGHT_PAREN,  // )
    INT,          // 123, +123, -123
    STRING,       // "string"
    DOT,          // .
    FLOAT,        // 123.567, 123., .567, +123.4, -.123
    NIL,          // nil, #f
    T,            // t, #t
    QUOTE,        // '
    SYMBOL,       // 以上不是都是symbol(區分大小寫)
    ATOM,         // 原子
    PROCEDURE,    // 自訂議程式
    INTERNAL,     // 預設
    
} ; // TokenType

enum ErrorType {

  REQ_ATOM,             // 未預期的token
  REQ_RIGHT_P,          // 缺少右括號
  EOLENCT,              // 未關閉字串
  EOFENCT,              // 檔案結束
  EXIT,                 // 出去
  UNBOUND_SYMBOL,       // 未綁定的符號
  DEFINE_FORMAT,        // define格式錯誤
  INCORRECT_NUM_ARGS ,  // 參數量錯誤
  INCORRECT_TYPE,       // 非列表的對象
  DIVISION_BY_ZERO,     // 除數為0
  NO_RETURN_VALUE,      // 條件語句無返回值
  COND_FORMAT,          // cond格式錯誤
  APPLY_NON_FUNCTION,   // 執行的對象不是函數
  LEVEL,                // func在不對的level
  NON_LIST,             // 不是標準的list

} ;

// 文法 :
// <S-exp> ::= <ATOM>
//            | LEFT_PAREN<S-exp>{<S-exp>}[DOT<S-exp>]RIGHT_PAREN
//            | QUITE<S-exp>
// <ATOM> ::= SYMBOL | INT | FLOAT | STRING | NIL | LEFT_PAREN RIGHT_PAREN
// | : 或, {} : 出現0次或多次, [] : 出現0次或1次

typedef char * CharPtr ; // the strings of all tokens are to be stored in the heap

struct Token {  // AST Tree

    string tokenStr = "" ;
    TokenType type = INTERNAL ;
    int line = 1 ;
    int col = 0 ;
    Token * left = NULL ;
    Token * right = NULL ;

} ; // Token

typedef Token * TokenPtr ;

static int uTestNum = -1 ;

int gLine = 1 ;
int gColume = 0 ;

bool isPrint = false ;
bool ProgEnd = false ;

void printToken( TokenPtr node ) ;
void prettyPrint( TokenPtr node, int indent ) ;
void PrintSExp( TokenPtr node ) ;

class Exception : public exception {

    private :

      string msg ;
      string func ;
      TokenPtr msgNode ;
      ErrorType type ;
      int line ;
      int col ;

    public :

      Exception( ErrorType type, string s, int l, int c ) {

        this->type = type ;
        msg = s ;
        line = l ;
        col = c ;

      }

      Exception( ErrorType type, TokenPtr node, int l, int c ) {

        this->type = type ;
        msgNode = node ;
        line = l ;
        col = c ;

      }

      Exception( ErrorType type, TokenPtr node, string f, int l, int c ) {

        this->type = type ;
        msgNode = node ;
        func = f ;
        line = l ;
        col = c ;

      }

      ErrorType getType() { return type ; }

      void what() {

        if ( type == EOFENCT )
          cout << "ERROR (no more input) : END-OF-FILE encountered" ;
        else if ( type == EOLENCT ) {
          cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " << line << " Column " << col + 1 << endl ;
        } // else if
        else if ( type == REQ_ATOM ) {
          cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " << line << " Column " << col ;
          cout << " is >>" << msg << "<<" << endl ;
        } // else if
        else if ( type == REQ_RIGHT_P ) {
          cout << "ERROR (unexpected token) : ')' expected when token at Line " << line << " Column " << col ;
          cout << " is >>" << msg << "<<" << endl ;
        } // else if
        else if ( type == UNBOUND_SYMBOL ) 
          cout << "ERROR (unbound symbol) : " << msg << endl ;
        else if ( type == INCORRECT_NUM_ARGS ) 
          cout << "ERROR (incorrect number of arguments) : " << msg << endl ;
        else if ( type == DEFINE_FORMAT ) {
          cout << "ERROR (DEFINE format) : " ;
          PrintSExp( msgNode ) ;
        } // else if
        else if ( type == INCORRECT_TYPE ) {
          cout << "ERROR (" << func << " with incorrect argument type) : " ;
          PrintSExp( msgNode ) ;
        } // else if
        else if ( type == DIVISION_BY_ZERO )
          cout << "ERROR (division by zero) : " << msg << endl ;
        else if ( type == NO_RETURN_VALUE ) {
          cout << "ERROR (no return value) : " ;
          PrintSExp( msgNode ) ;
        } // else if
        else if ( type == COND_FORMAT ) {
          cout << "ERROR (COND format) : " ;
          PrintSExp( msgNode ) ;
        } // else if
        else if ( type == APPLY_NON_FUNCTION ) {
          cout << "ERROR (attempt to apply non-function) : " ;
          PrintSExp( msgNode ) ;
        } // else if
        else if ( type == LEVEL )
          cout << "ERROR (level of " << msg << ")" << endl ;
        else if ( type == NON_LIST ) {
          cout << "ERROR (non-list) : " ;
          PrintSExp( msgNode ) ;
        } // else if

      } // what

} ; // Exception

class Scanner {

    private :

      char ch ;
      string token ;
      string peekedToken ;
      bool isPeek ;

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
      
        if ( ch >= 65 && ch <= 90 )  return true ;        // 判斷是否是A~Z
        else if ( ch >= 97 && ch <= 122 )  return true ;  // 判斷是否是a~z
        return false ;
      
      } // IsLetter()
      
      string GetStringConst() {
      
        string strconst = "" ;
        char before = '\0' ;
      
        strconst = strconst + '\"' ;
        ch = getChar() ;
        ch = cin.peek() ;

        while ( ch != '\"' || before == '\\' ) {

          if ( ch == '\n' ) throw Exception( EOLENCT, "", gLine, gColume ) ;

          ch = getChar() ;
      
          if ( ch == '\\' ) {
            before = ch ;
            ch = cin.peek() ;
            if ( ch == 'n' ) {
              ch = getChar() ;
              strconst = strconst + '\n' ;
            } // if
            else if ( ch == 't' ) {
              ch = getChar() ;
              strconst = strconst + '\t' ;
            } // else if
            else if ( ch == '\"' ) {
              ch = getChar() ;
              strconst = strconst + '\"' ;
            } // else if
            else if ( ch == '\\' ) {
              ch = getChar() ;
              strconst = strconst + '\\' ;
            } // else if
            else {
              strconst = strconst + before ;
            } // else
          } // if
          else {
            strconst = strconst + ch ;
          } // else
      
          before = ch ;
          ch = cin.peek() ;
        } // while
      
        ch = getChar() ;
        strconst = strconst + '\"' ;
        
        return strconst ;
      
      } // GetStringConst()
      
      void CheckSymbol( string & str ) {
        // 檢查是否有NIL, T, QUOTE
      
        if ( str == "#t"  || str == "t" ) str = "#t"  ;
        else if ( str == "#f" ||  str == "nil"  || str == "()" ) str = "nil" ;
        else if ( str == "'" ) str = "quote" ;
      
      } // CheckSymbol 
      
      string GetSymbol() {
      
        string str = "" ;
        
        ch = cin.peek() ;
      
        while ( !isspace(ch) && ch != '(' && ch != ')' && ch != '\'' && ch != '\"' && ch != ';' ) {

          ch = getChar() ;
      
          str = str + ch ;
          ch = cin.peek() ;
      
        } // while
      
        CheckSymbol( str ) ;
      
        return str ;
      
      } // GetSymbol

    public :

      Scanner() {

        ch = '\0' ;
        peekedToken = "" ;
        isPeek = false ;
        Init() ;

      } // Scanner

      void Init() {

        token = "" ;
        gLine = 1 ;
        gColume = 0 ;

      } // Init

      void SkipWhiteSpaces( ) {

        ch = cin.peek() ;

        while ( isspace(ch) ) {
          ch = getChar() ;
          ch = cin.peek() ;
        } // while

      } // SkipWhiteSpaces()
      
      void SkipLineComment( ) {
      
        ch = cin.peek() ;

        while ( ch != '\n' ) {
          ch = getChar() ;
          ch = cin.peek() ;
        } // while
      
      } // SkipLineComment()

      char getChar() {

        ch = cin.get() ;

        if ( ch == EOF ) throw Exception( EOFENCT, "", gLine, gColume ) ;

        if ( ch == '\n' ) {

          gLine++ ;
          gColume = 0 ;

          if ( isPrint ) {

            gLine = 1 ;
            isPrint = false ;

          } // if

        } // if
        else gColume++ ;

        return ch ;

      } // getChar

      string getToken() {

        if ( isPeek ) {

          isPeek = false ;
          return peekedToken ;

        } // if

        ch = cin.peek() ;

        while ( isspace(ch) || ch == ';' ) {

            ch = getChar() ;

            if ( isspace(ch) )  SkipWhiteSpaces() ; // 跳過空字元
            else if ( ch == ';' )  SkipLineComment() ;  // 跳過line comment
        
            ch = cin.peek() ;
        
        } // while
        
        if ( ch == '\"' ) token = GetStringConst() ;  // 判斷是不是string
        else if ( ch == '(' ) {
            ch = getChar() ;
            token = "(" ;
        } // else if
        else if ( ch == ')' ) {
            ch = getChar() ;
            token = ")" ;
        } // else if
        else if ( ch == '\'' ) {
            ch = getChar() ;
            token = "\'" ;
        } // else if
        else token = GetSymbol( ) ;  // 除了不是string一律讀進來

        isPrint = false ;
        
        return token ;

      } // getToken

      string peekToken() {

        if ( !isPeek ) {

          peekedToken = getToken() ;
          isPeek = true ;

        } // if

        return peekedToken ;

      } // peekToken

      TokenType GetType( string token ) {
        // 去判斷所有類別
      
        if ( token == "#t" ) return T ;
        else if ( token == "nil" ) return NIL ;
        else if ( token == "quote" ) return QUOTE ;
        else if ( token == "." ) return DOT ;
        else if ( token[0] == '\"' && token[token.length()-1] == '\"' ) return STRING ;
        else {
      
          int isNum = 0 ;    // 有可能沒出現數字
          int isPlus = 0 ;   // 紀錄出現'+'的次數
          int isMinus = 0 ;  // 紀錄出現'-'的次數
          int isDot = 0 ;    // 紀錄出現'.'的次數
      
          for ( int i = 0 ; i < token.length() ; i++ ) {
      
            if ( IsLetter(token[i]) ) return SYMBOL ;
            else if ( IsDigit(token[i]) ) isNum++ ;
            else if ( token[i] == '+' ) isPlus++ ;
            else if ( token[i] == '-' ) isMinus++ ;
            else if ( token[i] == '.' ) isDot++ ;
      
          } // for
      
          // 若isDot == 0 就是int, 而isDot == 1 就是float
          // 而'+'跟'-'出現的次數不可以超過2次以上
          if ( isNum > 0 && isDot == 0 && isPlus + isMinus < 2 ) return INT ;
          else if ( isNum > 0 && isDot == 1 && isPlus + isMinus < 2 ) return FLOAT ;
          else return SYMBOL ;
      
        } // else
      
      } // GetType

} ; // Scanner

Scanner scanner = Scanner() ;

class Parser {

    private :

      TokenPtr SExp_ATOM() {

        TokenPtr node = new Token() ;
        
        node->tokenStr = scanner.getToken() ;
        node->type = scanner.GetType( node->tokenStr ) ;
        node->line = gLine ;
        node->col = gColume ;

        return node ;

      } // SExp_ATOM

      TokenPtr SExp_Quote() {

        scanner.getToken() ;

        TokenPtr node = new Token() ;

        node->left = new Token() ;
        node->left->tokenStr = "quote" ;
        node->left->type = scanner.GetType( "quote" ) ;
        node->left->line = gLine ;
        node->left->col = gColume ;

        node->right = new Token() ;
        node->right->right = NULL ;
        node->right->left = ReadSExp() ;

        return node ;

      } // SExp_Quote

      TokenPtr SExp_List() {

        scanner.getToken() ;
        TokenPtr node = new Token() ;

        string token = scanner.peekToken() ;

        if ( token == ")" ) {

          scanner.getToken() ;
          node->tokenStr = "nil" ;
          node->type = NIL ;
          node->line = gLine ;
          node->col = gColume ;
          return node ;

        } // if

        node->left = ReadSExp() ;

        TokenPtr parent = node ;
        token = scanner.peekToken() ;

        while ( token != ")" ) {

          if ( token == "." ) {

            scanner.getToken() ;
            parent->right = ReadSExp() ;
            string close = scanner.getToken() ;

            if ( close != ")" ) throw Exception( REQ_RIGHT_P, close, gLine, gColume ) ;

            return node ;

          } // if

          parent->right = new Token() ;
          parent = parent->right ;
          parent->left = ReadSExp() ;

          token = scanner.peekToken() ;

        } // while

        scanner.getToken() ;
        parent->right = NULL ;

        return node ;

      } // SExp_List

    public :

      bool isLeaf( TokenPtr node ) {

        if ( node->left == NULL && node->right == NULL ) return true;
        return false ;

      } // isLeaf

      TokenPtr ReadSExp() {

        string token = scanner.peekToken() ;
        TokenPtr node = new Token() ;

        if ( token == "(" ) node = SExp_List() ;
        else if ( token == "\'" ) node = SExp_Quote() ;
        else if ( token == "." ) {
          scanner.getToken() ;
          throw Exception( REQ_ATOM, ".", gLine, gColume ) ;
        } // else if
        else if ( token == ")" ) {
          scanner.getToken() ;
          throw Exception( REQ_ATOM, ")", gLine, gColume ) ;
        } // else if
        else node = SExp_ATOM() ;

        return node ;

      } // ReadSExp

} ; // Parser

Parser parser = Parser() ;

class Evaluation {

  private :

    map<string, TokenPtr> mEnv ;
    int level ;

    bool isFunc( string s ) {

      if ( s == "define" ) return true ;
      else if ( s == "quote") return true ;
      else if ( s == "list" ) return true ;
      else if ( s == "cons" ) return true ;
      else if ( s == "car" ) return true ;
      else if ( s == "cdr" ) return true ;
      else if ( s == "pair?" ) return true ;
      else if ( s == "null?" ) return true ;
      else if ( s == "integer?" ) return true ;
      else if ( s == "real?" ) return true ;
      else if ( s == "number?" ) return true ;
      else if ( s == "string?" ) return true ;
      else if ( s == "boolean?" ) return true ;
      else if ( s == "symbol?" ) return true ;
      else if ( s == "+" ) return true ;
      else if ( s == "-" ) return true ;
      else if ( s == "*" ) return true ; 
      else if ( s == "/" ) return true ;
      else if ( s == "not" ) return true ;
      else if ( s == ">" ) return true ;
      else if ( s == "<" ) return true ;
      else if ( s == ">=" ) return true ;
      else if ( s == "<=" ) return true ;
      else if ( s == "=" ) return true ;
      else if ( s == "string-append" ) return true ;
      else if ( s == "string>?" ) return true ;
      else if ( s == "string<?" ) return true ;
      else if ( s == "string=?" ) return true ;
      else if ( s == "eqv?" ) return true ;
      else if ( s == "equal?" ) return true ;
      else if ( s == "if" ) return true ;
      else if ( s == "cond" ) return true ;
      else if ( s == "begin" ) return true ;
      else if ( s == "and" ) return true ;
      else if ( s == "or" ) return true ;
      else if ( s == "exit" ) return true ;
      else if ( s == "atom?" ) return true ;
      else if ( s == "clean-environment" ) return true ;
 
      return false ;

    } // isCommand

    TokenPtr Define( TokenPtr node ) {
      // 定義

      if ( node == NULL || node->left == NULL || node->right == NULL || ( node->right->right != NULL && node->right->right->type != NIL ) || node->left->type != SYMBOL ) {

        TokenPtr cur = new Token() ;
        cur->left = new Token() ;
        cur->right = node ;
        cur->left->tokenStr = "define" ;
        cur->left->type = SYMBOL ;
        throw Exception( DEFINE_FORMAT, cur, 0, 0 ) ;

      } // if

      if ( level != 1 ) throw Exception( LEVEL, "DEFINE", 0, 0 ) ;

      string name = node->left->tokenStr ;

      if ( isFunc( name ) ) {

        TokenPtr cur = new Token() ;
        cur->left = new Token() ;
        cur->right = node ;
        cur->left->tokenStr = "define" ;
        cur->left->type = SYMBOL ;
        throw Exception( DEFINE_FORMAT, cur, 0, 0 ) ;

      } // if

      TokenPtr value = EvalSExp( node->right->left ) ;

      mEnv[name] = value ;

      TokenPtr result = new Token() ;
      result->tokenStr = name + " defined" ;
      result->type = SYMBOL ;

      return result ;

    } // Define

    TokenPtr CleanEnv( TokenPtr node ) {
      // 把環境變數清除

      if ( node != NULL ) throw Exception( INCORRECT_NUM_ARGS, "claen-environment", 0, 0 ) ;

      mEnv.clear() ;

      TokenPtr result = new Token() ;
      result->tokenStr = "environment cleaned" ;
      result->type = STRING ;
      return result ;

    } // ClaenEnv

    TokenPtr Cons( TokenPtr node ) {
      // 把 dot 加進去

      if ( node == NULL || node->left == NULL || node->right == NULL || node->right->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "cons", 0, 0 ) ;

      if ( node->left->left != NULL && node->left->left->tokenStr == "clean-environment" ) throw Exception( LEVEL, "CLEAN-ENVIRONMENT", 0, 0 ) ;
      // ( cons (clean-env) ... )

      TokenPtr first = EvalSExp( node->left ) ;
      TokenPtr second = EvalSExp( node->right->left ) ;

      TokenPtr result = new Token() ;

      result->left = first ;
      result->right = second ;

      return result ;

    } // Cons

    TokenPtr List( TokenPtr node ) {

      if ( node == NULL ) {

        TokenPtr nilNode = new Token() ;
        nilNode->tokenStr = "nil" ;
        nilNode->type = NIL ;
        return nilNode ;

      } // if

      TokenPtr result = new Token() ;
      TokenPtr cur = result ;
      TokenPtr tmp = node ;

      while ( tmp != NULL ) {

        cur->left = EvalSExp( tmp->left ) ;

        tmp = tmp->right ;

        if ( tmp != NULL ) {

          cur->right = new Token() ;
          cur = cur->right ;

        } // if
        else {

          TokenPtr nilNode = new Token() ;
          nilNode->tokenStr = "nil" ;
          nilNode->type = NIL ;
          cur->right = nilNode ;

        } // else

      } // while

      return result ;

    } // List

    TokenPtr Quote( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "quote", 0, 0 ) ;

      return node->left ;

    } // Quote

    TokenPtr Car( TokenPtr node ) {
      // 找 dot 左邊的節點

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "car", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      if ( target == NULL || parser.isLeaf( target ) ) throw Exception( INCORRECT_TYPE, target, "car", 0, 0 ) ;

      return target->left ;

    } // Car

    TokenPtr Cdr( TokenPtr node ) {
      // 找 dot 右邊的節點

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "cdr", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      if ( target == NULL || parser.isLeaf( target ) ) throw Exception( INCORRECT_TYPE, target, "cdr", 0, 0 ) ;

      return target->right ;

    } // Cdr

    TokenPtr Pair( TokenPtr node ) {
      // left or right 其中一個不是 NULL 就是 true
      // 若是 ATOM or nil 就是 false

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "pair?", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target != NULL && ( target->left != NULL || target->right != NULL ) ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;
        
      } // else

      return result ;

    } // Pair

    TokenPtr Null( TokenPtr node ) {
      // 判斷是不是NIL

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "null?", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == NIL ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Null

    TokenPtr Integer( TokenPtr node ) {
      // 判斷是不是INT

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "integer?", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == INT ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Integer

    TokenPtr Real( TokenPtr node, string func ) {
      // 判斷是不是INT or FLOAT

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, func, 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == INT || target->type == FLOAT ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Real

    TokenPtr String( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "string?", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == STRING ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // String

    TokenPtr Boolean( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "boolean?", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == NIL || target->type == T ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Boolean

    TokenPtr Symbol( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "symbol?", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == SYMBOL ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Symbol

    TokenPtr Add( TokenPtr node ) {

      TokenPtr cur = node ;
      double sum = 0 ;
      bool isFloat = false ;
      int count = 0 ;

      while ( cur != NULL ) {

        TokenPtr val = EvalSExp( cur->left ) ;
        count ++ ;

        if ( val->type == INT ) sum += atoi( val->tokenStr.c_str() ) ;
        else if ( val->type == FLOAT ) {

          sum += atof( val->tokenStr.c_str() ) ;
          isFloat = true ;

        } // else if
        else throw Exception( INCORRECT_TYPE, val, "+", 0, 0 ) ;

        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, "+", 0, 0 ) ;

      TokenPtr result = new Token() ;

      if ( isFloat ) {

        result->tokenStr = to_string( sum ) ;
        result->type = FLOAT ;

      } // if
      else {

        result->tokenStr = to_string( (int)sum ) ;
        result->type = INT ;

      } // else

      return result ;

    } // Add

    TokenPtr Minus( TokenPtr node ) {

      TokenPtr cur = node ;
      bool isFloat = false ;
      double result = 0.0 ;
      int count = 0 ;

      while ( cur != NULL ) {

        TokenPtr val = EvalSExp( cur->left ) ;
        count ++ ;

        if ( val->type == INT ) {

          if ( count == 1 ) result = atoi( val->tokenStr.c_str() ) ;
          else result -= atoi( val->tokenStr.c_str() ) ;

        } // if
        else if ( val->type == FLOAT ) {

          isFloat = true ;
          if ( count == 1 ) result = atof( val->tokenStr.c_str() ) ;
          else result -= atof( val->tokenStr.c_str() ) ;

        } // else if
        else throw Exception( INCORRECT_TYPE, val, "-", 0, 0 ) ;

        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, "-", 0, 0 ) ;

      TokenPtr res = new Token() ;

      if ( isFloat ) {

        res->tokenStr = to_string( result ) ;
        res->type = FLOAT ;

      } // if
      else {

        res->tokenStr = to_string( (int)result ) ;
        res->type = INT ;

      } // else

      return res ;

    } // Minus

    TokenPtr Multi( TokenPtr node ) {

      TokenPtr cur = node ;
      double product = 1.0 ;
      bool isFloat = false ;
      int count = 0 ;

      while ( cur != NULL ) {

        TokenPtr val = EvalSExp( cur->left ) ;
        count ++ ;

        if ( val->type == INT ) product *= atoi( val->tokenStr.c_str() ) ;
        else if ( val->type == FLOAT ) {

          isFloat = true ;
          product *= atof( val->tokenStr.c_str() ) ;

        } // else if
        else throw Exception( INCORRECT_TYPE, val, "*", 0, 0 ) ;

        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, "*", 0, 0 ) ;

      TokenPtr result = new Token() ;

      if ( isFloat ) {

        result->tokenStr = to_string( product ) ;
        result->type = FLOAT ;

      } // if
      else {

        result->tokenStr = to_string( (int)product ) ;
        result->type = INT ;

      } // else

      return result ;

    } // Multi

    TokenPtr Divide( TokenPtr node ) {

      TokenPtr cur = node ;
      bool isFloat = false ;
      double result = 0.0 ;
      int count = 0 ;

      if ( cur == NULL ) throw Exception( INCORRECT_NUM_ARGS, "/", 0, 0 ) ;

      TokenPtr val = EvalSExp( cur->left ) ;
      count ++ ;

      if ( val->type == INT ) result = atoi( val->tokenStr.c_str() ) ;
      else if ( val->type == FLOAT ) {

        isFloat = true ;
        result = atof( val->tokenStr.c_str() ) ;

      } // else if
      else throw Exception( INCORRECT_TYPE, val, "/", 0, 0 ) ;

      cur = cur->right ;

      while ( cur != NULL ) {

        val = EvalSExp( cur->left ) ;
        count ++ ;

        double op ;
        if ( val->type == INT ) op = atoi( val->tokenStr.c_str() ) ;
        else if ( val->type == FLOAT ) {

          isFloat = true ;
          op = atof( val->tokenStr.c_str() ) ;

        } // else if
        else throw Exception( INCORRECT_TYPE, val, "/", 0, 0 ) ;

        if ( op == 0.0 ) throw Exception( DIVISION_BY_ZERO, "/", 0, 0 ) ;

        result /= op ;
        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, "/", 0, 0 ) ;

      TokenPtr res = new Token() ;

      if ( isFloat ) {

        res->tokenStr = to_string( result ) ;
        res->type = FLOAT ;

      } // if
      else {

        res->tokenStr = to_string( (int)result ) ;
        res->type = INT ;

      } // else

      return res ;

    } // Divide

    TokenPtr Not( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "not", 0, 0 ) ;

      TokenPtr target = EvalSExp( node->left ) ;

      TokenPtr result = new Token() ;

      if ( target->type == NIL ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Not

    TokenPtr Compare( TokenPtr node, string op ) {

      vector<double> values ;
      int count = 0 ;
      bool isFloat = false ;

      TokenPtr cur = node ;

      while ( cur != NULL ) {

        TokenPtr val = EvalSExp( cur->left ) ;
        count ++ ;

        if ( val->type == INT ) values.push_back( atoi(val->tokenStr.c_str()) ) ;
        else if ( val->type == FLOAT ) {

          isFloat = true ;
          values.push_back( atof(val->tokenStr.c_str()) ) ;

        } // else if
        else throw Exception( INCORRECT_TYPE, val, op, 0, 0 ) ;

        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, op, 0, 0 ) ;

      TokenPtr result = new Token() ;
      bool ok = true ;

      for ( int i = 0 ; i < values.size() - 1 ; i++ ) {

        double num1 = values[i] ;
        double num2 = values[i + 1] ;

        if ( op == ">" ) {
          if ( !( num1 > num2 ) ) ok = false ;
        } // if
        else if ( op == ">=" ) {
          if ( !( num1 >= num2 ) ) ok = false ;
        } // else if
        else if ( op == "<" ) {
          if ( !( num1 < num2 ) ) ok = false ;
        } // else if
        else if ( op == "<=" ) {
          if ( !( num1 <= num2 ) ) ok = false ;
        } // else if
        else if ( op == "=" ) {
          if ( !( num1 == num2 ) ) ok = false ;
        } // else if

        if ( !ok ) break ;

      } // for

      if ( ok ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Compare

    TokenPtr StrAppend( TokenPtr node ) {

      TokenPtr cur = node ;
      int count = 0 ;
      string resultStr = "" ;

      while ( cur != NULL ) {

        TokenPtr val = EvalSExp( cur->left ) ;
        count ++ ;

        if ( val->type != STRING ) ; // error msg

        string str = val->tokenStr ;
        if ( str.length() >= 2 && str.front() == '"' && str.back() == '"' ) // 去掉頭尾的 "
          str = str.substr( 1, str.length() - 2 ) ;
        else throw Exception( INCORRECT_TYPE, val, "string-append", 0, 0 ) ;

        resultStr += str ;
        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, "string-append", 0, 0 ) ;

      TokenPtr result = new Token() ;
      result->tokenStr = "\"" + resultStr + "\"" ;
      result->type = STRING ;
      
      return result ;

    } // StrAppend

    TokenPtr StrCompare( TokenPtr node, string op ) {

      vector<string> values ;
      int count = 0 ;

      TokenPtr cur = node ;

      while ( cur !=NULL ) {

        TokenPtr val = EvalSExp( cur->left ) ;
        count ++ ;

        if ( val->type != STRING ) ; // error msg

        string str = val->tokenStr ;
        if ( str.length() >= 2 && str.front() == '"' && str.back() == '"' ) // 去掉頭尾的 "
          str = str.substr( 1, str.length() - 2 ) ;
        else throw Exception ( INCORRECT_TYPE, val, op, 0, 0 ) ;

        values.push_back( str ) ;
        cur = cur->right ;

      } // while

      if ( count < 2 ) throw Exception( INCORRECT_NUM_ARGS, op, 0, 0 ) ;

      TokenPtr result = new Token() ;
      bool ok = true ;

      for ( int i = 0 ; i < values.size() - 1 ; i++ ) {

        string str1 = values[i] ;
        string str2 = values[i + 1] ;

        if ( op == "string>?" ) {
          if ( !( str1 > str2 ) ) ok = false ;
        } // if
        else if ( op == "string<?" ) {
          if ( !( str1 < str2 ) ) ok = false ;
        } // else if
        else if ( op == "string=?" ) {
          if ( !( str1 == str2 ) ) ok = false ;
        } // else if

        if ( !ok ) break ;

      } // for

      if ( ok ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else 

      return result ;

    } // StrCompare

    TokenPtr Eqv( TokenPtr node ) {

      if ( node == NULL || node->right == NULL || node->right->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "eqv?", 0, 0 ) ;

      TokenPtr a = EvalSExp( node->left ) ;
      TokenPtr b = EvalSExp( node->right->left ) ;

      TokenPtr result = new Token() ;

      if ( a == b ) { // 相同的記憶體位置

        result->tokenStr = "#t" ;
        result->type = T ;
        return result ;

      } // if

      if ( parser.isLeaf( a ) && parser.isLeaf( b ) && a->type == b->type && a->tokenStr == b->tokenStr ) { // 都是ATOM且值一樣

        if ( a->type == STRING || b->type == STRING ) {

          result->tokenStr = "nil" ;
          result->type = NIL ;
          return result ;

        } // if

        result->tokenStr = "#t" ;
        result->type = T ;
        return result ;

      } // if

      result->tokenStr = "nil" ;
      result->type = NIL ;
      return result ;

    } // Eqv

    bool isEqual( TokenPtr a, TokenPtr b ) {

      if ( a == NULL && b == NULL ) return true ;

      if ( a == NULL || b == NULL ) return false ;

      if ( parser.isLeaf( a ) && parser.isLeaf( b ) ) return ( a->type == b->type && a->tokenStr == b->tokenStr ) ;

      if (parser.isLeaf( a ) || parser.isLeaf( b ) ) return false ;

      return isEqual( a->left, b->left ) && isEqual( a->right, b->right ) ;

    } // isEqal

    TokenPtr Equal( TokenPtr node ) {

      if ( node == NULL || node->right == NULL || node->right->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "equal?", 0, 0 ) ;

      TokenPtr a = EvalSExp( node->left ) ;
      TokenPtr b = EvalSExp( node->right->left ) ;

      TokenPtr result = new Token() ;

      if ( isEqual( a, b ) ) {

        result->tokenStr = "#t" ;
        result->type = T ;
         
      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Eqal

    bool isTrue( TokenPtr t ) { return !( t->type == NIL ) ; } // isTrue

    TokenPtr If( TokenPtr node ) {

      if ( node == NULL || node->left == NULL ) throw Exception( INCORRECT_NUM_ARGS, "if", 0, 0 ) ;

      TokenPtr cond = EvalSExp( node->left ) ;

      TokenPtr thenNode = NULL ;
      TokenPtr elseNode = NULL ;

      if ( node->right != NULL ) {

        thenNode = node->right->left ;

        if ( node->right->right != NULL ) elseNode = node->right->right->left ;

        if ( node->right->right != NULL && node->right->right->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "if", 0, 0 ) ;

      } // if
      else throw Exception( INCORRECT_NUM_ARGS, "if", 0, 0 ) ;

      if ( isTrue(cond) ) return EvalSExp( thenNode ) ;
      else {

        if ( elseNode != NULL ) return EvalSExp( elseNode ) ;
        else {

          TokenPtr cur = new Token() ;
          cur->left = new Token() ;
          cur->right = node ;
          cur->left->tokenStr = "if" ;
          cur->left->type = SYMBOL ;
          throw Exception( NO_RETURN_VALUE, cur, 0, 0 ) ;

        } // else

      } // else

    } // If

    bool isCondFormat( TokenPtr node ) {

      TokenPtr cur = node ;

      while ( cur != NULL ) {

        TokenPtr clause = cur->left ;

        if ( clause == NULL || clause->left == NULL || clause->right == NULL ) return false ;

        cur = cur->right ;

      } // while

      return true ;

    } // isCondFormat

    TokenPtr Cond( TokenPtr node ) {

      if ( node == NULL || !isCondFormat( node ) ) {

        TokenPtr tmp = new Token() ;
        tmp->left = new Token() ;
        tmp->right = node ;
        tmp->left->tokenStr = "cond" ;
        tmp->left->type = SYMBOL ;
        throw Exception( COND_FORMAT, tmp, 0, 0 ) ;

      } // if

      TokenPtr cur = node ;

      while ( cur != NULL ) {

        TokenPtr clause = cur->left ;
        TokenPtr test = clause->left ;

        bool isLastClause = ( cur->right == NULL ) ;
        bool isVailElse = ( test->type == SYMBOL && test->tokenStr == "else" && isLastClause ) ;

        if ( isVailElse ) {

          TokenPtr expr = clause->right ;
          TokenPtr result = NULL ;

          while ( expr != NULL ) {

            result = EvalSExp( expr->left ) ;
            expr = expr->right ;

          } // while

          return result ;

        } // if

        TokenPtr res = EvalSExp( test ) ;

        if ( isTrue( res ) ) {

          TokenPtr expr = clause->right ;
          TokenPtr result = NULL ;

          while ( expr != NULL ) {

            result = EvalSExp( expr->left ) ;
            expr = expr->right ;

          } // while

          return result ;

        } // if

        cur = cur->right ;

      } // while

      TokenPtr tmp = new Token() ;
      tmp->left = new Token() ;
      tmp->right = node ;
      tmp->left->tokenStr = "cond" ;
      tmp->left->type = SYMBOL ;
      throw Exception( NO_RETURN_VALUE, tmp, 0, 0 ) ;

    } // Cond

    TokenPtr Begin( TokenPtr node ) {

      if ( node == NULL ) throw Exception( INCORRECT_NUM_ARGS, "begin", 0, 0 ) ;

      TokenPtr result = NULL ;
      TokenPtr cur = node ;

      while ( cur != NULL ) {

        result = EvalSExp( cur->left ) ;
        cur = cur->right ;

      } // while

      return result ;

    } // Begin

    TokenPtr And( TokenPtr node ) {

      if ( node == NULL || node->right == NULL ) throw Exception( INCORRECT_NUM_ARGS, "and", 0 , 0 ) ;

      TokenPtr cur = node ;
      TokenPtr result = NULL ;

      while( cur != NULL ) {

        result = EvalSExp( cur->left ) ;

        if ( result == NULL || result->type == NIL ) {

          TokenPtr f = new Token() ;
          f->tokenStr = "nil" ;
          f->type = NIL ;
          return f ;

        } // if

        cur = cur->right ;

      } // while

      return result ;

    } // And

    TokenPtr Or( TokenPtr node ) {

      if ( node == NULL || node->right == NULL ) throw Exception( INCORRECT_NUM_ARGS, "or", 0 , 0 ) ;

      TokenPtr cur = node ;

      while ( cur != NULL ) {

        TokenPtr result = EvalSExp( cur->left ) ;

        if ( result != NULL && result->type != NIL ) return result ;

        cur = cur->right ;

      } // while

      TokenPtr f = new Token() ;
      f->tokenStr = "nil" ;
      f->type = NIL ;
      return f ;

    } // Or

    TokenPtr Exit ( TokenPtr node ) {

      if ( level != 1 ) throw Exception( LEVEL, "EXIT", 0, 0 ) ;

      if ( node != NULL && node->type != NIL ) throw Exception( INCORRECT_NUM_ARGS, "exit", 0, 0 ) ;

      TokenPtr cur = new Token() ;
      cur->left = new Token() ;
      cur->left->tokenStr = "exit" ;
      cur->left->type = SYMBOL ;
      cur->right = node ;

      ProgEnd = true ;

      return cur ;

    } // Exit

    TokenPtr Atom( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || node->right != NULL ) throw Exception( INCORRECT_NUM_ARGS, "atom?", 0, 0 ) ;
      
      TokenPtr target = EvalSExp( node->left ) ;
      TokenPtr result = new Token() ;

      if ( target == NULL || parser.isLeaf( target ) ) {

        result->tokenStr = "#t" ;
        result->type = T ;

      } // if
      else {

        result->tokenStr = "nil" ;
        result->type = NIL ;

      } // else

      return result ;

    } // Atom

    TokenPtr Lambda( TokenPtr node ) {

      if ( node == NULL || node->left == NULL || parser.isLeaf( node->left ) ) ; // error

      TokenPtr proc = new Token() ;
      proc->tokenStr = "#<procedure lambda>" ;
      proc->type = PROCEDURE ;
      proc->left = node->left ;
      proc->right = node->right ;

      return proc ;

    } // Lambda

  public :

    Evaluation() {

      level = 0 ;

    }

    bool isList( TokenPtr node ) {

      TokenPtr cur = node ;

      while ( cur != NULL ) {

        if ( parser.isLeaf( cur ) && cur->type != NIL ) return false ;

        cur = cur->right ;

      } // while

      return true ;

    } // isList

    TokenPtr EvalSExp( TokenPtr node ) {

      level ++ ;

      if ( node == NULL ) {

        level -- ;
        return NULL ;

      } // if

      if ( parser.isLeaf( node ) ) { // 處理ATOM

        if ( node->type == SYMBOL ) {

          if ( mEnv.count( node->tokenStr ) > 0 && !isFunc( node->tokenStr ) ) return mEnv[node->tokenStr] ;
          else if ( isFunc( node->tokenStr ) ) {

            string str = "" ;
            str = "#<procedure " + node->tokenStr + ">" ;

            TokenPtr result = new Token() ;

            result->tokenStr = str ;
            result->type = SYMBOL ;

            return result ;

          } // else if
          else throw Exception( UNBOUND_SYMBOL, node->tokenStr, 0, 0 ) ;

        } // if
        
        return node ;

      } // if

      if ( !parser.isLeaf( node->left ) ) node->left = EvalSExp( node->left ) ;

      // func 開始
      string func = node->left->tokenStr ;

      if ( node->left->type == SYMBOL ) {

        if ( mEnv.count( func ) > 0 ) { // 把被定義的func變回正常的func

          TokenPtr cur = mEnv[func] ;

          if ( cur->tokenStr.find( "#<procedure " ) != string::npos ) func = cur->tokenStr.substr( 12, cur->tokenStr.length() - 13 ) ; // 把 #<procedure 和 > 刪掉
          else func = cur->tokenStr ;

          node->left->tokenStr = func ;

        } // if
        else if ( func.find( "#<procedure " ) != string::npos ) func = func.substr( 12, func.length() - 13 ) ;
        else if ( !isFunc( func ) ) throw Exception( UNBOUND_SYMBOL, func, 0, 0 ) ;

      } // if

      if ( func == "define" ) return Define( node->right ) ;
      else if ( func == "clean-environment" ) return CleanEnv( node->right ) ;
      else if ( func == "cons" ) return Cons( node->right ) ;
      else if ( func == "list" ) return List( node->right ) ;
      else if ( func == "quote" ) return Quote( node->right ) ;
      else if ( func == "car" ) return Car( node->right ) ;
      else if ( func == "cdr" ) return Cdr( node->right ) ;
      else if ( func == "pair?" ) return Pair( node->right ) ;
      else if ( func == "null?" ) return Null( node->right ) ;
      else if ( func == "integer?" ) return Integer( node->right ) ;
      else if ( func == "real?" || func == "number?" ) return Real( node->right, func ) ;
      else if ( func == "string?" ) return String( node->right ) ;
      else if ( func == "boolean?" ) return Boolean( node->right ) ;
      else if ( func == "symbol?" ) return Symbol( node->right ) ;
      else if ( func == "+" ) return Add( node->right ) ;
      else if ( func == "-" ) return Minus( node->right ) ;
      else if ( func == "*" ) return Multi( node->right ) ;
      else if ( func == "/" ) return Divide( node->right ) ;
      else if ( func == "not" ) return Not( node->right ) ;
      else if ( func == ">" || func == "<" || func == ">=" || func == "<=" || func == "=" ) return Compare( node->right, func ) ;
      else if ( func == "string-append" ) return StrAppend( node->right ) ;
      else if ( func == "string>?" || func == "string<?" || func == "string=?" ) return StrCompare( node->right, func ) ;
      else if ( func == "eqv?" ) return Eqv( node->right ) ;
      else if ( func == "equal?" ) return Equal( node->right ) ;
      else if ( func == "if" ) return If( node->right ) ;
      else if ( func == "cond" ) return Cond( node->right ) ;
      else if ( func == "begin" ) return Begin( node->right ) ;
      else if ( func == "and" ) return And( node->right ) ;
      else if ( func == "or" ) return Or( node->right ) ;
      else if ( func == "exit" ) return Exit( node->right ) ;
      else if ( func == "atom?" ) return Atom( node->right ) ;
      else throw Exception( APPLY_NON_FUNCTION, node->left, 0, 0 ) ;

    } // EvalSExp

    void Init() {

      level = 0 ;

    } // Init

} ; // Evaluate

Evaluation eval = Evaluation() ;

void printToken( TokenPtr node ) {

  if ( node->type == INT ) cout << atoi(node->tokenStr.c_str()) << endl ;
  else if ( node->type == FLOAT ) cout << fixed << setprecision(3) << atof(node->tokenStr.c_str()) << endl ;
  else if ( node->type != INTERNAL ) cout << node->tokenStr << endl ;
      
} // PrintToken

void prettyPrint( TokenPtr node, int indent ) {

  if ( node == NULL ) return ;

  if ( node->left == NULL && node->right == NULL ) {
    printToken( node ) ;
    return ;
  } // if

  cout << "( " ;

  prettyPrint( node->left, indent + 2 ) ;

  TokenPtr current = node->right ;

  while ( current != NULL ) {

    if ( current->left == NULL && current->right == NULL ) {
      if ( current->tokenStr != "nil" ) {
        for ( int i = 0 ; i < indent + 2 ; i++ ) cout << " " ;
        cout << "." << endl ;
        for ( int i = 0 ; i < indent + 2 ; i++ ) cout << " " ;
        printToken( current ) ;
      } // if
      break ;
    } // if

    for ( int i = 0 ; i < indent + 2 ; i++ ) cout << " " ;
    prettyPrint( current->left, indent + 2 ) ;
    current = current->right ;
  } // if

  for ( int i = 0 ; i < indent ; i++ ) cout << " " ;
  cout << ")" << endl ;

} // prettyPrint

void PrintSExp( TokenPtr node ) {

  isPrint = true ;

  if ( node->left == NULL && node->right == NULL ) printToken( node ) ; // print SATRING, INT, FLOAT...
  else {

    if ( ProgEnd ) throw Exception( EXIT, "", gLine, gColume ) ;

    prettyPrint( node, 0 ) ; // print (...)

  } // else

} // PrintSExp

int main() {

    string tmp ;
    TokenPtr inSExp = NULL ;
    TokenPtr resultSExp = NULL ;
    
    cout << "Welcome to OurScheme!" << endl ;
    
    cin >> uTestNum ;

    while ( true ) {

        try {
            cout << "\n> " ;
            inSExp = parser.ReadSExp() ;
            if ( !parser.isLeaf( inSExp ) && !eval.isList( inSExp ) ) throw Exception( NON_LIST, inSExp, 0, 0 ) ;
            resultSExp = eval.EvalSExp( inSExp ) ;
            if ( resultSExp != NULL ) PrintSExp( resultSExp ) ;
        } catch( Exception & e ) {
          if ( e.getType() == EXIT ) break ;
          else {
            e.what() ;
            isPrint = true ;
            if ( e.getType() == EOFENCT ) break ;
            else if ( e.getType() == EOLENCT || e.getType() == REQ_ATOM || e.getType() == REQ_RIGHT_P ) {
              try {
                getline(cin, tmp) ;
              } catch ( exception & e ) {
                break ;
              } // catch
            } // else 
          } // else 
        } // catch

        scanner.Init() ;
        eval.Init() ;

    } // while

    cout << "\nThanks for using OurScheme!" ;

    return 0 ;
    
} // main