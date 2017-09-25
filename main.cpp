# include <cctype>
# include <cstdlib>
# include <cstring>
# include <cstdio>

# include <iostream>
# include <map>
# include <string>
# include <vector>
# include <sstream>
# include <exception>
# include <iomanip>

# define cmdNum 54

using namespace std;

bool gProgEnd = false, gVerbos = true ;
int gLine = 1, gCol = 0;

enum TokenType {
  EMPTY, NIL, INT, FLOAT, DOT, T, STRING, SHARP,
  OPERATOR, LEFT_PAREN, RIGHT_PAREN, SYMBOL,
  CONS, LIST, QUOTE, DEFINE, CAR, CDR, ISPAIR, ISLIST, ISATOM, ISNULL,
  ISINT, ISREAL, ISNUM, ISSTR, ISBOOL, ISSYM, ADD, SUB, MULT, DIV,
  NOT, AND, OR, BIGG, BIGEQ, SML, SMLEQ, EQ, STRAPP, STRBIG,
  STRSML, STREQL, ISEQV, ISEQL, BEGIN, IF, COND, LET, LAMBDA, PRINT, READ,
  WRITE, EVAL, DSPSTR, NEWLINE, SYMTOSTR, NUMTOSTR, SET, CEATEOBJ, ISERROBJ,
  VERBOS, ISVERBOS, EXIT, CLEAN, USRFUNC, ERROBJ
};

enum ExcpType { NULLERR, EOFENCT, REQ_ATOM, REQ_RIGHT_P, EOLENCT, NONFUNC, NONLIST, WRONGTYPE, DEFERR,
                WRONGARGNUM, UNBOND, DEFLVERR, CLEANLVERR, EXITLVERR, DIVZRO, NORET, CNDERR, LETERR,
                LAMBERR, UNBONDPARA, UNBONDTEST, UNBONDCOND, NORET_B, SETERR
};

enum TPtrType { LEFT, RIGHT, PRE } ;

class Token ;
typedef Token* TokenPtr ;

string gInternalCmd[ cmdNum ] = { "cons", "list", "quote", "define", "car", "cdr",
                         "pair?", "list?", "atom?", "null?", "integer?", "real?", "number?",
                         "string?", "boolean?", "symbol?", "+", "-", "*",
                         "/", "not", "and", "or", ">", ">=", "<", "<=",
                         "=", "string-append", "string>?", "string<?", "string=?", "eqv?", "equal?",
                         "begin", "if", "cond", "let", "lambda", "print", "read", "write", "eval",
                         "display-string", "newline", "symbol->string", "number->string", "set!",
                         "create-error-object", "error-object?", "verbose", "verbose?",
                         "exit", "clean-environment"
                                } ;

map<string,TokenPtr> gDefineMap ;

TokenPtr EvalSExp( Token * root, int level, map<string,TokenPtr> * localDefMap ) ;
void PrintSExp( Token * tokenList, int level, bool isLeft, bool inErr, stringstream & ss ) ;

class OurFunction ;

union OurNumber {
  int integer ;
  float real ;
};

class Scanner {
public:
  bool mHasOutput ;
  bool mAcceptType[ 15 ] ;
  friend class OurException;
  Scanner() {
    gLine = 1;
    gCol = 0;
    mHasOutput = false ;
    for ( int i = 0 ; i < 15 ; ++ i )
      mAcceptType[ i ] = true ;
  } // Scanner()

  char Getchar() {
    char c = getchar();
    if ( c == -1 )
      return '\r';
    ++gCol;
    return c;
  } // Getchar()

  void Putback( char ch ) {
    cin.putback( ch );
    --gCol;
  } // Putback()

private:
  string mLineBuf;
};

Scanner gSc;

class Token {
public:
  int mType;
  bool mIsQuoted ;
  string mContent;
  TokenPtr mPtrList[ 3 ] ;
  OurFunction * mFunc ;
  Token() {
    mType = EMPTY;
    mContent = "";
    mIsQuoted = false ;
    for ( int i = 0 ; i < 3 ; ++ i )
      mPtrList[ i ] = NULL ;
    mFunc = NULL ;
  } // Token()

  Token( int type, string content ) {
    mType = type;
    mContent = content;
    mIsQuoted = false ;
    for ( int i = 0 ; i < 3 ; ++ i )
      mPtrList[ i ] = NULL ;
    mFunc = NULL ;
  } // Token()
};

class Scanner;
class OurException {
public:
  int mEType;
  string mErrMsg;

  OurException() {
    mEType = EMPTY;
    mErrMsg = "Why do you want to throw an empty exception?";
  } // OurException()

  OurException( Token * token, int type, char deli ) {
    stringstream ss;
    mEType = type ;
    if ( type == REQ_ATOM ) {
      if ( token->mContent == ")" )
        ++ gCol ;
      ss << "ERROR (unexpected token) : "
         << "atom or '(' expected when token at Line "
         << gLine
         << " Column "
         << gCol - token->mContent.size()
         << " is >>" << token->mContent << "<<" ;
      if ( deli != '\n' ) {
        char c = 0 ;
        do {
          c = getchar() ;
        } while (c != '\n' && c != -1 ) ;
      } // if

      mErrMsg = ss.str();

      gLine = 1;
      gCol = 0;
    } // if
    else if ( type == REQ_RIGHT_P ) {
      ss << "ERROR (unexpected token) : "
         << "\')\' expected when token at Line "
         << gLine
         << " Column "
         << gCol - token->mContent.size() + 1
         << " is >>" << token->mContent << "<<" ;
      char c = 0 ;
      do {
        c = getchar() ;
      } while ( c != '\n' && c != -1 ) ;

      mErrMsg = ss.str();

      gLine = 1;
      gCol = 0;
    } // else if
  } // OurException()

  OurException( int type ) {
    mEType = type ;
    if ( type == EOFENCT ) {
      mErrMsg = "ERROR (no more input) : END-OF-FILE encountered";
      gProgEnd = true;
    } // if
    else if ( type == DEFLVERR ) {
      mErrMsg = "ERROR (level of DEFINE)" ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == CLEANLVERR ) {
      mErrMsg = "ERROR (level of CLEAN-ENVIRONMENT)" ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == EXITLVERR ) {
      mErrMsg = "ERROR (level of EXIT)" ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == EOLENCT ) {
      stringstream ss ;
      ss << "ERROR (no closing quote) : END-OF-LINE encountered at Line "
         << gLine
         << " Column "
         << gCol ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
  } // OurException()

  OurException( int type, string content ) {
    mEType = type ;
    if ( type == NONFUNC ) {
      stringstream ss ;
      ss << "ERROR (attempt to apply non-function) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // if
    else if ( type == NONLIST ) {
      stringstream ss ;
      ss << "ERROR (non-list) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == WRONGARGNUM ) {
      stringstream ss ;
      ss << "ERROR (incorrect number of arguments) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == UNBOND ) {
      stringstream ss ;
      ss << "ERROR (unbound symbol) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == DEFERR ) {
      stringstream ss ;
      ss << "ERROR (DEFINE format) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == DIVZRO ) {
      stringstream ss ;
      ss << "ERROR (division by zero) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == NORET || type == NORET_B ) {
      stringstream ss ;
      ss << "ERROR (no return value) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 1 ;
    } // else if
    else if ( type == CNDERR ) {
      stringstream ss ;
      ss << "ERROR (COND format) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 1 ;
    } // else if
    else if ( type == LETERR ) {
      stringstream ss ;
      ss << "ERROR (LET format) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == SETERR ) {
      stringstream ss ;
      ss << "ERROR (SET! format) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == LAMBERR ) {
      stringstream ss ;
      ss << "ERROR (LAMBDA format) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == UNBONDPARA ) {
      stringstream ss ;
      ss << "ERROR (unbound parameter) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == UNBONDTEST ) {
      stringstream ss ;
      ss << "ERROR (unbound test-condition) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
    else if ( type == UNBONDCOND ) {
      stringstream ss ;
      ss << "ERROR (unbound condition) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // else if
  } // OurException()

  OurException( int type, string errOperator, string content ) {
    mEType = type ;
    if ( type == WRONGTYPE ) {
      stringstream ss ;
      ss << "ERROR (" << errOperator << " with incorrect argument type) : " << content ;
      mErrMsg = ss.str() ;
      gLine = 1 ;
      gCol = 0 ;
    } // if
  } // OurException()
};

void PrintSExp( Token * tokenList, int level, bool isLeft, bool inErr, stringstream & ss ) ;

class OurFunction {
public:

  OurFunction() {
    mFunctionName = "lambda" ;
  } // OurFunction()

  OurFunction( string usrDefName ) {
    mFunctionName = usrDefName ;
  } // OurFunction()

  void SetArg( string argName ) {
    mArgList.push_back( argName ) ;
    return ;
  } // SetArg()

  void SetFunction( TokenPtr funcRoot ) {
    mFunctionBody.push_back( funcRoot ) ;
    return ;
  } // SetFunction()

  void SetName( string newName ) {
    mFunctionName = newName ;
    return ;
  } // SetName()

  string GetName() {
    return mFunctionName ;
  } // GetName()

  TokenPtr Evaluate( vector<TokenPtr> & argValues, int level, map<string,TokenPtr> * localDefMap ) {
    // cout << mFunctionName << " " << argValues.size() << endl ;
    // return new Token( STRING, "YEE" ) ;
    // let us check the function arg number first
    if ( mArgList.size() != argValues.size() ) {
      throw OurException( WRONGARGNUM, mFunctionName ) ;
    } // if

    for ( int i = 0 ; i < argValues.size() ; ++ i ) {
      try {
        argValues.at( i ) = EvalSExp( argValues.at( i ), level + 1, localDefMap ) ;
      } catch ( OurException e ) {
        if ( e.mEType == NORET ) {
          stringstream ss ;
          PrintSExp( argValues.at( i ), 1, true, true, ss ) ;
          throw OurException( UNBONDPARA, ss.str() ) ;
        } // if
        else
          throw ;
      } // catch()
    } // for

    // Then, set all value to right argument
    map<string,TokenPtr> * argMap = new map<string,TokenPtr>() ;
    map<string,TokenPtr> & argMapRef = *argMap ;
    for ( int i = 0 ; i < mArgList.size() ; ++ i ) {
      if ( argValues.at( i ) == NULL )
        exit( 0 ) ;
      argMapRef[ mArgList.at( i ) ] = argValues.at( i ) ;
    } // for


    // cout << argValues.at( 0 )->mContent << endl ;

    // stringstream ss ;
    // PrintSExp( mFunctionBody.at( 0 ), 1, true, true, ss ) ;
    // cout << ss.str() ;

    TokenPtr result = NULL ;
    for ( int i = 0 ; i < mFunctionBody.size() ; ++ i ) {
      try {
        result = EvalSExp( mFunctionBody.at( i ), level, argMap ) ;
      } catch( OurException e ) {
        if ( i == mFunctionBody.size() - 1 && e.mEType == NORET ) {
          stringstream ss ;
          PrintSExp( mFunctionBody.at( i ), 1, true, true, ss ) ;
          throw OurException( NORET, ss.str() ) ;
        } // if
        else {
          if ( e.mEType != NORET )
            throw ;
        } // else
      } // catch()
    } // for


    return result ;
  } // Evaluate()

private:
  string mFunctionName ;
  vector<string> mArgList ;
  vector<TokenPtr> mFunctionBody ;
};

Token * ReadToken() {
  /*
  ReadToken() is a function witch can read a token
  and determine its type then return it to ReadSExp
  */

  /*
  DOT, OPERATOR and SHARP are mid-type, they are only use in ReadToken(),
  and should not appear in other function.
  */

  bool encounterTEnd = false, inComment = false;
  char buf = '\0';
  Token * resultToken = new Token();
  while ( !encounterTEnd ) {
    buf = gSc.Getchar();
    if ( resultToken->mType == EMPTY ) {
      if ( isalpha( buf ) ) {
        if ( buf == 't' )
          resultToken->mType = T;
        else
          resultToken->mType = SYMBOL;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mType = INT;
      } // else if
      else if ( buf == '(' || buf == ')' ) {
        if ( buf == '(' )
          resultToken->mType = LEFT_PAREN;
        else
          resultToken->mType = RIGHT_PAREN;
        encounterTEnd = true;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = DOT;
      } // else if
      else if ( buf == '\'' ) {
        resultToken->mType = QUOTE;
        encounterTEnd = true;
      } // else if
      else if ( buf == '\"' ) {
        resultToken->mType = STRING;
      } // else if
      else if ( buf == '#' ) {
        resultToken->mType = SHARP;
      } // else if
      else if ( buf == '+' || buf == '-' ) {
        resultToken->mType = OPERATOR;
      } // else if
      else if ( isspace( buf ) ) {
        if ( buf == '\n' ) {
          if ( gSc.mHasOutput ) {
            gLine = 1 ;
            gSc.mHasOutput = false ;
          } // if
          else
            ++gLine ;
          gCol = 0 ;
        } // if
        else if ( buf == '\r' )
          throw OurException( EOFENCT );

        // encounterTEnd = true ;
      } // else if
      else if ( buf == ';' ) {
        char t = 0 ;
        do {
          t = gSc.Getchar() ;
          if ( t == '\r' )
            throw OurException( EOFENCT ) ;
        } while ( t != '\n' ) ;

        gSc.Putback( '\n' );
      } // else if
      else
        resultToken->mType = SYMBOL;

      if ( !isspace( buf ) && buf != ';' )
        resultToken->mContent += buf;
    } // if
    else if ( resultToken->mType == SHARP ) {
      if ( isalpha( buf ) ) {
        if ( buf == 'f' )
          resultToken->mType = NIL;
        else if ( buf == 't' )
          resultToken->mType = T;
        else
          resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '(' || buf == ')' || buf == '\''
                || buf == '\"' || isspace( buf ) || buf == ';' ) {
        gSc.Putback( buf );
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == OPERATOR ) {
      if ( isalpha( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mType = INT;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '(' || buf == ')' || buf == '\''
                || buf == '\"' || isspace( buf ) || buf == ';' ) {
        gSc.Putback( buf );
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = FLOAT ;
        resultToken->mContent += buf;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == NIL ) {
      if ( isalpha( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '\"' || isspace( buf ) || buf == '\'' ||
                buf == '(' || buf == ')' || buf == ';' ) {
        // if ( !gSc.mAcceptType[ NIL ] )
        //     throw OurException( buf, UNEXCEPT );
        // else {
        gSc.Putback( buf );
        encounterTEnd = true;
        // } // else
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == DOT ) {
      if ( isalpha( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mType = FLOAT;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '\'' || buf == '\"' || isspace( buf ) ||
                buf == '(' || buf == ')' || buf == ';' ) {
        if ( !gSc.mAcceptType[ DOT ] )
          throw OurException( resultToken, REQ_ATOM, buf );
        else {
          gSc.Putback( buf );
          encounterTEnd = true;
        } // else
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == INT ) {
      if ( isalpha( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '(' || buf == ')' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = FLOAT;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '\'' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '\"' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( isspace( buf ) ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == ';' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == FLOAT ) {
      if ( isalpha( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '(' || buf == ')' || buf == '\''
                || buf == '\"' || isspace( buf ) || buf == ';' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == T ) {
      if ( isalpha( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // if
      else if ( isdigit( buf ) ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '(' || buf == ')' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '.' ) {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else if
      else if ( buf == '\'' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '\"' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( isspace( buf ) ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == ';' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == STRING ) {
      if ( buf != '\"' ) {
        bool fakeEsc = false ;
        char escChar = '\0' ;
        if ( buf == '\n' )
          throw OurException( EOLENCT );
        else if ( buf == '\r' ) {
          ++ gCol ;
          throw OurException( EOLENCT ) ; // what?!
        } // else if
        else if ( buf == '\\' ) {
          escChar = gSc.Getchar() ;
          if ( escChar == 'n' )
            buf = '\n' ;
          else if ( escChar == 't' )
            buf = '\t' ;
          else if ( escChar == '\"' )
            buf = '\"' ;
          else if ( escChar != '\\' )
            fakeEsc = true ;
        } // else if

        resultToken->mContent += buf;
        if ( fakeEsc )
          resultToken->mContent += escChar ;
      } // if
      else {
        encounterTEnd = true;
        if ( buf == '\"' )
          resultToken->mContent += buf;
      } // else
    } // else if
    else if ( resultToken->mType == SYMBOL ) {
      if ( buf == '(' || buf == ')' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '\'' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == '\"' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( buf == ';' ) {
        gSc.Putback( buf );
        encounterTEnd = true;
      } // else if
      else if ( !isspace( buf ) ) {
        resultToken->mContent += buf;
      } // if
      else {
        encounterTEnd = true;
        gSc.Putback( buf );
      } // else
    } // else if
  } // while

  if ( !gSc.mAcceptType[ resultToken->mType ] )
    throw OurException( resultToken, REQ_ATOM, buf ) ;

  return resultToken;
} // ReadToken()

void ReadSExp( int level, Token * root ) {
  Token * thisNode = NULL ;
  bool hasRightParen = true ;
  if ( root->mType == QUOTE ) {
    root->mType = LEFT_PAREN ;
    root->mContent = "(" ;
    root->mPtrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
    root->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
    gSc.mAcceptType[ DOT ] = false ; // ( '.
    gSc.mAcceptType[ RIGHT_PAREN ] = false ; // ( ')
    root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = ReadToken() ;
    if ( root->mPtrList[ RIGHT ]->mPtrList[ LEFT ]->mType == LEFT_PAREN ) // ( ( quote . ( ( ...
      ReadSExp( level + 1, root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] ) ;
    else if ( root->mPtrList[ RIGHT ]->mPtrList[ LEFT ]->mType == QUOTE ) {
      ReadSExp( level + 1, root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] ) ;
    } // else if

    hasRightParen = false ;
  } // if

  if ( root->mPtrList[ LEFT ] == NULL ) {
    // (
    gSc.mAcceptType[ DOT ] = false ; // ( '.
    gSc.mAcceptType[ RIGHT_PAREN ] = true ; // ()
    root->mPtrList[ LEFT ] = ReadToken() ;
    // cout << root->mPtrList[ LEFT ]->mContent << endl ;
    if ( root->mPtrList[ LEFT ]->mType == LEFT_PAREN ) // ( ( ...
      ReadSExp( level + 1, root->mPtrList[ LEFT ] ) ;
    else if ( root->mPtrList[ LEFT ]->mType == QUOTE ) { // ( '
      root->mPtrList[ LEFT ] = new Token( LEFT_PAREN, "(" ) ;
      root->mPtrList[ LEFT ]->mPtrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
      root->mPtrList[ LEFT ]->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      thisNode = root->mPtrList[ LEFT ]->mPtrList[ RIGHT ] ;
      // ( ( quote . (
      gSc.mAcceptType[ DOT ] = false ; // ( '.
      gSc.mAcceptType[ RIGHT_PAREN ] = false ; // ( ')
      thisNode->mPtrList[ LEFT ] = ReadToken() ; // ( ( quote . ( a . nil )
      if ( thisNode->mPtrList[ LEFT ]->mType == LEFT_PAREN ) // ( ( quote . ( ( ...
        ReadSExp( level + 1, thisNode->mPtrList[ LEFT ] ) ;
      else if ( thisNode->mPtrList[ LEFT ]->mType == QUOTE ) {
        ReadSExp( level + 1, thisNode->mPtrList[ LEFT ] ) ;
      } // else if
    } // else if
    else if ( root->mPtrList[ LEFT ]->mType == RIGHT_PAREN ) {
      // ()
      root->mType = NIL ;
      root->mContent = "nil" ;
      root->mPtrList[LEFT] = NULL ;
      hasRightParen = false ;
      return ;
    } // else if
  } // if

  if ( root->mPtrList[ RIGHT ] == NULL ) {
    // ( a
    gSc.mAcceptType[ DOT ] = true ; // ( a .
    gSc.mAcceptType[ RIGHT_PAREN ] = true ; // ( a )
    root->mPtrList[ RIGHT ] = ReadToken() ;
    // cout << "LEFT " << root->mPtrList[ RIGHT ]->mContent << endl ;
    if ( root->mPtrList[ RIGHT ]->mType == DOT ) {
      // ( a .
      gSc.mAcceptType[ DOT ] = false ; // ( '.
      gSc.mAcceptType[ RIGHT_PAREN ] = false ; // ( ')
      root->mPtrList[ RIGHT ] = ReadToken() ;
      // cout << root->mPtrList[ RIGHT ]->mContent << endl ;
      if ( root->mPtrList[ RIGHT ]->mType == LEFT_PAREN )  { // ( a . (
        root->mPtrList[ RIGHT ]->mContent = ".(" ;
        ReadSExp( level + 1, root->mPtrList[ RIGHT ] ) ;
      } // if
      else if ( root->mPtrList[ RIGHT ]->mType == QUOTE ) {
        // ( a . '
        root->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
        root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
        ReadSExp( level + 1, root->mPtrList[ RIGHT ] ) ;
        hasRightParen = false ;
      } // else if

      gSc.mAcceptType[ DOT ] = true ; // it's a trap!
      gSc.mAcceptType[ RIGHT_PAREN ] = true ; // ( ')
    } // if
    else if ( root->mPtrList[ RIGHT ]->mType == QUOTE ) {
      root->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ; // ( a .(
      root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = new Token( LEFT_PAREN, "(" ) ; // ( a .( (
      thisNode = root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] ;
      thisNode->mPtrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
      thisNode->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      gSc.mAcceptType[ DOT ] = false ; // ( '.
      gSc.mAcceptType[ RIGHT_PAREN ] = false ; // ( ')
      thisNode->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = ReadToken() ;
      if ( thisNode->mPtrList[ RIGHT ]->mPtrList[ LEFT ]->mType == LEFT_PAREN
           || thisNode->mPtrList[ RIGHT ]->mPtrList[ LEFT ]->mType == QUOTE )
        ReadSExp( level + 1, thisNode->mPtrList[ RIGHT ]->mPtrList[ LEFT ] ) ;

      ReadSExp( level + 1, root->mPtrList[ RIGHT ] ) ;
      hasRightParen = false ;
    } // else if
    else if ( root->mPtrList[ RIGHT ]->mType != RIGHT_PAREN ) {
      thisNode = root->mPtrList[ RIGHT ] ;
      root->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = thisNode ;
      if ( thisNode->mType == LEFT_PAREN )
        ReadSExp( level + 1, thisNode ) ;
      ReadSExp( level + 1, root->mPtrList[ RIGHT ] ) ;
      // ( a b c d )
      // when d in, d will steal root's ')'
      // so we must told root it's R-Paren had been stolen
      hasRightParen = false ;
    } // else if
  } // if

  while ( hasRightParen && root->mPtrList[ RIGHT ]->mType != RIGHT_PAREN ) {
    Token * dummy = ReadToken() ;
    if ( dummy->mType != RIGHT_PAREN )
      throw OurException( dummy, REQ_RIGHT_P, '\0' ) ;
    else
      return ;
  } // while

  return ;
} // ReadSExp()

bool IsCallable( Token * t ) {
  for ( int i = 0 ; i < cmdNum ; ++ i ) {
    if ( t->mContent == gInternalCmd[ i ] && !t->mIsQuoted ) {
      t->mType = i + 12 ;
      return true ;
    } // if
  } // for

  if ( t->mType >= CONS && t->mType <= USRFUNC )
    return true ;
  else
    return false ;
} // IsCallable()

void FixToken( Token * t ) {
  if ( t->mType == NIL ) {
    t->mContent = "nil";
  } // if
  else if ( t->mType == QUOTE )
    t->mContent = "quote";
  else if ( t->mType == T ) {
    t->mContent = "#t";
  } // else if
  else if ( t->mType == INT ) {
    stringstream ss;
    ss << atoi( t->mContent.c_str() );
    t->mContent = ss.str();
  } // else if
  else if ( t->mType == FLOAT ) {
    char floatBuf[32] = { '\0' };
    // The floating number will round off here!!!
    bool realFloat = false ;
    for ( int i = 0 ; i < t->mContent.size() ; ++ i ) {
      if ( isdigit( t->mContent.at( i ) ) )
        realFloat = true ;
    } // for

    if ( realFloat ) {
      stringstream ss ;
      ss << fixed << setprecision( 3 ) << atof( t->mContent.c_str() ) ;
      /*
      string tempFloat = ss.str() ;
      for ( int i = 0 ; i < tempFloat.size() ; ++ i ) {
        if ( tempFloat.at( i ) == '.' ) {
          for ( int j = 0 ; j < tempFloat.size() ; ++ j ) {
            if ( j == 4 ) {
              int value = tempFloat.at( i + j ) - '0' ;
              if ( value >= 5 )
                tempFloat.at( i + j - 1 ) = tempFloat.at( i + j - 1 ) + 1 ;
              while ( tempFloat.size() !=  i + 4 )
                tempFloat = tempFloat.substr( 0, tempFloat.size()-1 ) ;
            } // if
          } // for
        } // if
      } // for
      */
      t->mContent = ss.str() ;
    } // if
    else {
      t->mType = SYMBOL ;
    } // else
  } // else if
  else if ( t->mType == RIGHT_PAREN ) {
    t->mContent = "nil" ;
    t->mType = NIL ;
  } // else if
  else if ( t->mType == SYMBOL ) {
    if ( t->mContent == "nil" )
      t->mType = NIL;
    else if ( t->mContent == "quote" )
      t->mType = QUOTE ;
  } // else if
  else if ( t->mPtrList[ LEFT ] != NULL && t->mPtrList[ RIGHT ] == NULL ) {
    t->mPtrList[ RIGHT ] = new Token( NIL, "nil" ) ;
  } // else if
} // FixToken()

void GetOriginalName( Token * t ) {
  if ( t->mType >= CONS && t->mType <= CLEAN )
    t->mContent = gInternalCmd[ t->mType - 12 ] ;
} // GetOriginalName()

void PrintSExp( Token * tokenList, int level, bool isLeft, bool inErr, stringstream & ss ) {
  if ( tokenList->mType == LEFT_PAREN && isLeft ) {
    ss << "( " ;
    if ( tokenList->mPtrList[ LEFT ] != NULL )
      PrintSExp( tokenList->mPtrList[ LEFT ], level + 1, true, inErr, ss ) ;
    if ( tokenList->mPtrList[ RIGHT ] != NULL && tokenList->mPtrList[ RIGHT ]->mType != RIGHT_PAREN )
      PrintSExp( tokenList->mPtrList[ RIGHT ], level, false, inErr, ss ) ;
    for ( int i = 0 ; i < ( level - 1 ) * 2 ; ++ i )
      ss << ' ' ;
    ss << ')' ;
    if ( level > 1 )
      ss << endl ;
  } // if
  else {
    if ( tokenList->mType == LEFT_PAREN && !isLeft ) {
      for ( Token * seek = tokenList ; seek != NULL
            && seek->mType != RIGHT_PAREN ; seek = seek->mPtrList[ RIGHT ] ) {
        if ( seek->mType == LEFT_PAREN && !isLeft ) {
          if ( seek->mPtrList[ LEFT ]->mType == LEFT_PAREN ) {
            for ( int i = 0 ; i < level * 2 ; ++ i )
              ss << ' ' ;
            PrintSExp( seek->mPtrList[ LEFT ], level + 1, true, inErr, ss ) ;
          } // if
          else {
            for ( int i = 0 ; i < level * 2 ; ++ i )
              ss << ' ' ;
            // if ( seek->mPtrList[ LEFT ]->mIsQuoted ) ss << "Q->" ;
            if ( !inErr && IsCallable( seek->mPtrList[ LEFT ] ) )
              ss << seek->mPtrList[ LEFT ]->mContent << endl ;
            else {
              GetOriginalName( seek->mPtrList[ LEFT ] ) ;
              ss << seek->mPtrList[ LEFT ]->mContent << endl ;
            } // else
          } // else
        } // if
        else if ( seek->mType != NIL ) {
          for ( int i = 0 ; i < level * 2 ; ++ i )
            ss << ' ' ;
          ss << '.' << endl ;
          for ( int i = 0 ; i < level * 2 ; ++ i )
            ss << ' ' ;
          // if ( seek->mIsQuoted ) ss << "Q->" ;
          if ( !inErr && IsCallable( seek ) )
            ss << seek->mContent << endl ;
          else {
            GetOriginalName( seek ) ;
            ss << seek->mContent << endl ;
          } // else
        } // else if
      } // for
    } // if
    else {
      if ( !isLeft && tokenList->mType != NIL ) {
        for ( int i = 0 ; i < level * 2 ; ++ i )
          ss << ' ' ;
        ss << '.' << endl ;
        for ( int i = 0 ; i < level * 2 ; ++ i )
          ss << ' ' ;
        // if ( tokenList->mIsQuoted ) ss << "Q->" ;
        if ( !inErr && IsCallable( tokenList ) )
          ss << tokenList->mContent << endl ;
        else {
          GetOriginalName( tokenList ) ;
          ss << tokenList->mContent << endl ;
        } // else
      } // if
      else if ( isLeft ) {
        // if ( tokenList->mIsQuoted ) ss << "Q->" ;
        if ( !inErr && IsCallable( tokenList ) )
          ss << tokenList->mContent ;
        else {
          GetOriginalName( tokenList ) ;
          ss << tokenList->mContent ;
        } // else

        if ( level > 1 )
          ss << endl ;
      } // else if

    } // else
  } // else
} // PrintSExp()

void DebugPrint( Token * root ) {
  FixToken( root ) ;
  // cout << "c " << root->mContent << endl ;
  if ( root->mPtrList[LEFT] != NULL ) {
    // cout << "L" << endl ;
    DebugPrint( root->mPtrList[ LEFT ] ) ;
  } // if

  if ( root->mPtrList[RIGHT] != NULL ) {
    // cout << "R" << endl ;
    DebugPrint( root->mPtrList[ RIGHT ] ) ;
  } // if

  // cout << "B" << endl ;
} // DebugPrint()

void Quote( Token * root ) {
  root->mIsQuoted = true ;
  if ( root->mPtrList[LEFT] != NULL )
    Quote( root->mPtrList[ LEFT ] ) ;
  if ( root->mPtrList[RIGHT] != NULL )
    Quote( root->mPtrList[ RIGHT ] ) ;
} // Quote()

void CheckAndGetListElement( Token * listHead, vector<Token*> & vt ) {
  for ( Token * seek = listHead->mPtrList[ RIGHT ] ; seek != NULL ; seek = seek->mPtrList[ RIGHT ] ) {
    if ( seek->mPtrList[ RIGHT ] == NULL && seek->mType != NIL ) {
      stringstream ss ;
      PrintSExp( listHead, 1, true, true, ss ) ;
      throw OurException( NONLIST, ss.str() ) ;
    } // if
    else if ( seek->mType != NIL ) {
      // cout << seek->mPtrList[ LEFT ]->mContent << endl ;
      /*
      if ( seek->mPtrList[ LEFT ]->mType == SYMBOL
           && gDefineMap.find( seek->mPtrList[ LEFT ]->mContent ) != gDefineMap.end() )
        seek->mPtrList[ LEFT ] = gDefineMap[ seek->mPtrList[ LEFT ]->mContent ]  ;
        // ( cons a<-[a defined arg] ... )
      */
      vt.push_back( seek->mPtrList[ LEFT ] ) ;
    } // else if
  } // for
} // CheckAndGetListElement()

void CheckAndGetPairElement( Token * listHead, vector<Token*> & vt ) {
  if ( listHead == NULL )
    exit( 0 ) ;
  for ( Token * seek = listHead ; seek != NULL ; seek = seek->mPtrList[ RIGHT ] ) {
    if ( seek->mPtrList[ RIGHT ] == NULL && seek->mType != NIL ) {
      stringstream ss ;
      PrintSExp( listHead, 1, true, true, ss ) ;
      // cout << seek->mContent << seek->mType << endl ;
      throw OurException( NONLIST, ss.str() ) ;
    } // if
    else if ( seek->mType != NIL ) {
      // cout << seek->mPtrList[ LEFT ]->mContent << endl ;
      /*
      if ( seek->mPtrList[ LEFT ]->mType == SYMBOL
           && gDefineMap.find( seek->mPtrList[ LEFT ]->mContent ) != gDefineMap.end() )
        seek->mPtrList[ LEFT ] = gDefineMap[ seek->mPtrList[ LEFT ]->mContent ]  ;
        // ( cons a<-[a defined arg] ... )
      */
      vt.push_back( seek->mPtrList[ LEFT ] ) ;
    } // else if
  } // for
} // CheckAndGetPairElement()

bool CheckDefine( Token * t ) {
  if ( t == NULL )
    exit( 0 ) ;
  if ( gDefineMap.find( t->mContent ) != gDefineMap.end() )
    return true ;
  else
    return false ;
} // CheckDefine()

Token * Cons( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 2 )
    throw OurException( WRONGARGNUM, "cons" ) ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    // cout << argList.at( i )->mContent << " " << argList.at( i )->mType << endl ;
    // if ( argList.at( i )->mType == LEFT_PAREN )
    try {
      argList.at( i ) = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()
    /*
    else if ( argList.at( i )->mType == SYMBOL ) {
      if ( CheckDefine( argList.at( i ) ) )
        argList.at( i ) = gDefineMap[ argList.at( i )->mContent ] ;
      else if ( !IsCallable( argList.at( i ) ) )
        throw OurException( UNBOND, argList.at( i )->mContent ) ;
    } // else if
    */
  } // for

  Token * result = new Token( LEFT_PAREN, "(" ) ;
  result->mPtrList[ LEFT ] = argList.at( 0 ) ;
  result->mPtrList[ RIGHT ] = argList.at( 1 ) ;

  return result ;
} // Cons()

Token * List( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.empty() )
    return new Token( NIL, "nil" ) ;
  else {
    Token * result = new Token( LEFT_PAREN, "(" ), * seek = result ;
    result->mPtrList[ LEFT ] = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
    for ( int i = 1 ; i < argList.size() ; ++ i ) {
      seek->mPtrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      // if ( argList.at( i )->mType == LEFT_PAREN )
      try {
        seek->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = EvalSExp( argList.at( i ),
                                                              level + 1, localDefMap ) ;
      } catch ( OurException e ) {
        if ( e.mEType == NORET ) {
          stringstream ss ;
          PrintSExp( argList.at( i ), 1, true, true, ss ) ;
          throw OurException( UNBONDPARA, ss.str() ) ;
        } // if
        else
          throw ;
      } // catch()

      /*
      else if ( argList.at( i )->mType == SYMBOL ) {
        if ( CheckDefine( argList.at( i ) ) )
          seek->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = gDefineMap[ argList.at( i )->mContent ] ;
        else if ( IsCallable( argList.at( i ) ) )
          seek->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = argList.at( i ) ;
        else
          throw OurException( UNBOND, argList.at( i )->mContent ) ;
      } // else if
      else
        seek->mPtrList[ RIGHT ]->mPtrList[ LEFT ] = argList.at( i ) ;
      */
      seek = seek->mPtrList[ RIGHT ] ;
    } // for

    seek->mPtrList[ RIGHT ] = new Token( NIL, "nil" ) ;
    return result ;
  } // else
} // List()

Token * Define( int level, Token * root, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( level > 0 )
    throw OurException( DEFLVERR ) ;

  if ( argList.size() < 2 ) {
    stringstream ss ;
    PrintSExp( root, 1, true, true, ss ) ;
    throw OurException( DEFERR, ss.str() ) ;
  } // if
  else {
    Token * symNeedDef = argList.at( 0 ) ;
    if ( ( symNeedDef->mType != SYMBOL && symNeedDef->mType != LEFT_PAREN )
         || symNeedDef->mIsQuoted || IsCallable( symNeedDef ) ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( DEFERR, ss.str() ) ;
    } // if
    else {
      if ( symNeedDef->mType == LEFT_PAREN ) {
        vector<TokenPtr> args ;
        CheckAndGetPairElement( symNeedDef, args ) ;

        TokenPtr funcNeedDef = args.at( 0 ) ;
        if ( funcNeedDef->mType == SYMBOL && !IsCallable( funcNeedDef ) ) {
          stringstream ss ;
          ss << "#<procedure " << funcNeedDef->mContent << ">" ;
          TokenPtr usrDeffunc = new Token( USRFUNC, ss.str() ) ;
          usrDeffunc->mFunc = new OurFunction( funcNeedDef->mContent ) ;

          for ( int i = 1 ; i < args.size() ; ++ i ) {
            // cout << args.at( i )->mContent << " " << args.at( i )->mType << endl ;
            if ( args.at( i )->mType == SYMBOL && !IsCallable( args.at( i ) ) )
              usrDeffunc->mFunc->SetArg( args.at( i )->mContent ) ;
            else {
              stringstream ss ;
              PrintSExp( root, 1, true, true, ss ) ;
              throw OurException( DEFERR, ss.str() ) ;
            } // else
          } // for

          for ( int i = 1 ; i < argList.size() ; ++ i ) {
            // TokenPtr evalArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
            // funcNode->mFunc->SetFunction( evalArg ) ;
            usrDeffunc->mFunc->SetFunction( argList.at( i ) ) ;
          } // for

          gDefineMap[ usrDeffunc->mFunc->GetName() ] = usrDeffunc ;

          ss.str( "" ) ;
          ss << usrDeffunc->mFunc->GetName() << " defined" ;
          if ( gVerbos )
            cout << ss.str() << endl ;
          return NULL ;
        } // if
        else {
          stringstream ss ;
          PrintSExp( root, 1, true, true, ss ) ;
          throw OurException( DEFERR, ss.str() ) ;
        } // else

        return new Token( SYMBOL, "new define!" ) ; // OOO defined.
      } // if
      else {
        if ( argList.size() != 2 ) {
          stringstream ss ;
          PrintSExp( root, 1, true, true, ss ) ;
          throw OurException( DEFERR, ss.str() ) ;
        } // if

        if ( argList.at( 1 )->mType == SYMBOL && ( !CheckDefine( argList.at( 1 ) )
                                                   && !IsCallable( argList.at( 1 ) ) ) )
          throw OurException( UNBOND, argList.at( 1 )->mContent ) ;

        Token * defResult = NULL ;
        try {
          defResult = EvalSExp( argList.at( 1 ), level + 1, localDefMap ) ;
        } catch ( OurException e ) {
          if ( e.mEType == NORET ) {
            stringstream ss ;
            PrintSExp( argList.at( 1 ), 1, true, true, ss ) ;
            throw OurException( NORET_B, ss.str() ) ;
          } // if
          else
            throw ;
        } // catch()

        if ( defResult->mType == USRFUNC && defResult->mContent == "#<procedure lambda>" )
          defResult->mFunc->SetName( "lambda" ) ;

        gDefineMap[ symNeedDef->mContent ] = defResult ;
        stringstream ss ;
        ss << symNeedDef->mContent << " defined" ;
        if ( gVerbos )
          cout << ss.str() << endl ;
        return NULL ; // OOO defined.
      } // else
    } // else
  } // else
} // Define()

Token * Car( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "car" ) ;
  try {
    argList.at( 0 ) = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( argList.at( 0 )->mType != LEFT_PAREN )
    throw OurException( WRONGTYPE, "car", argList.at( 0 )->mContent ) ;

  return argList.at( 0 )->mPtrList[ LEFT ] ;
} // Car()

Token * Cdr( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "cdr" ) ;

  try {
    argList.at( 0 ) = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( argList.at( 0 )->mType != LEFT_PAREN )
    throw OurException( WRONGTYPE, "cdr", argList.at( 0 )->mContent ) ;

  return argList.at( 0 )->mPtrList[ RIGHT ] ;
} // Cdr()

Token * IsPair( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "pair?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != LEFT_PAREN )
    return new Token( NIL, "nil" ) ;
  else {
    /*
    for ( ; pairRoot != NULL ; pairRoot = pairRoot->mPtrList[ RIGHT ] ) {
      if ( pairRoot->mType == NIL )
        return new Token( NIL, "nil" ) ;
    } // for
    */
    if ( pairRoot->mType == LEFT_PAREN )
      return new Token( T, "#t" ) ;
    else
      return new Token( NIL, "nil" ) ;
  } // else
} // IsPair()

Token * IsList( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "list?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType == NIL )
    return new Token( T, "#t" ) ;
  else if ( pairRoot->mType != LEFT_PAREN )
    return new Token( NIL, "nil" ) ;
  else {
    if ( pairRoot->mType == LEFT_PAREN ) {
      for ( ; pairRoot != NULL ; pairRoot = pairRoot->mPtrList[ RIGHT ] ) {
        if ( pairRoot->mPtrList[ RIGHT ] == NULL ) {
          if ( pairRoot->mType == NIL )
            return new Token( T, "#t" ) ;
          else
            return new Token( NIL, "nil" ) ;
        } // if
      } // for
    } // if
    else
      return new Token( NIL, "nil" ) ;
  } // else

  return new Token( T, "#t" ) ;
} // IsList()

Token * IsAtom( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "atom?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType == LEFT_PAREN )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsAtom()

Token * IsNull( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "null?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != NIL )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsNull()

Token * IsInt( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "integer?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != INT )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsInt()

Token * IsReal( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "real?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != INT && pairRoot->mType != FLOAT )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsReal()

Token * IsNum( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "number?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != INT && pairRoot->mType != FLOAT )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsNum()

Token * IsStr( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "string?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != STRING )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsStr()

Token * IsBool( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "boolean?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != T && pairRoot->mType != NIL )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsBool()

Token * IsSym( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "symbol?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( IsCallable( pairRoot ) )
    return new Token( T, "#t" ) ;
  else if ( pairRoot->mType != SYMBOL && !IsCallable( pairRoot ) )
    return new Token( NIL, "nil" ) ;
  else if ( IsCallable( pairRoot ) && !pairRoot->mIsQuoted )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsSym()

Token * Add( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  bool isFloat = false ;
  OurNumber result ;

  result.integer = 0 ;
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "+" ) ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    // Token * thisNum = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    Token * thisNum = NULL ;
    try {
      thisNum = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisNum->mType != INT && thisNum->mType != FLOAT ) {
      if ( thisNum->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisNum, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "+", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "+", thisNum->mContent ) ;
      } // else
    } // if
    else {
      if ( !isFloat && thisNum->mType == FLOAT ) {
        isFloat = true ;
        result.real = ( float ) result.integer ;
      } // if

      if ( isFloat ) {
        float trueNum = atof( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.real = trueNum ;
        else
          result.real += trueNum ;
      } // if
      else {
        int trueNum = atoi( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.integer = trueNum ;
        else
          result.integer += trueNum ;
      } // else
    } // else
  } // for

  stringstream ss ;
  if ( isFloat ) {
    ss << fixed << setprecision( 3 ) << result.real ;
    return new Token( FLOAT, ss.str() ) ;
  } // if
  else {
    ss << result.integer ;
    return new Token( INT, ss.str() ) ;
  } // else
} // Add()

Token * Sub( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  bool isFloat = false ;
  OurNumber result ;

  result.integer = 0 ;
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "-" ) ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisNum = NULL ;
    try {
      thisNum = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisNum->mType != INT && thisNum->mType != FLOAT ) {
      if ( thisNum->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisNum, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "-", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "-", thisNum->mContent ) ;
      } // else
    } // if
    else {
      if ( !isFloat && thisNum->mType == FLOAT ) {
        isFloat = true ;
        result.real = ( float ) result.integer ;
      } // if

      if ( isFloat ) {
        float trueNum = atof( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.real = trueNum ;
        else
          result.real -= trueNum ;
      } // if
      else {
        int trueNum = atoi( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.integer = trueNum ;
        else
          result.integer -= trueNum ;
      } // else
    } // else
  } // for

  stringstream ss ;
  if ( isFloat ) {
    ss << fixed << setprecision( 3 ) << result.real ;
    return new Token( FLOAT, ss.str() ) ;
  } // if
  else {
    ss << result.integer ;
    return new Token( INT, ss.str() ) ;
  } // else
} // Sub()

Token * Mult( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  // cout << "is *" << endl ;
  bool isFloat = false ;
  OurNumber result ;

  result.integer = 0 ;
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "*" ) ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisNum = NULL ;
    try {
      thisNum = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisNum->mType != INT && thisNum->mType != FLOAT ) {
      if ( thisNum->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisNum, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "*", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "*", thisNum->mContent ) ;
      } // else
    } // if
    else {
      if ( !isFloat && thisNum->mType == FLOAT ) {
        isFloat = true ;
        result.real = ( float ) result.integer ;
      } // if

      if ( isFloat ) {
        float trueNum = atof( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.real = trueNum ;
        else
          result.real *= trueNum ;
      } // if
      else {
        int trueNum = atoi( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.integer = trueNum ;
        else
          result.integer *= trueNum ;
      } // else
    } // else
  } // for

  stringstream ss ;
  if ( isFloat ) {
    ss << fixed << setprecision( 3 ) << result.real ;
    return new Token( FLOAT, ss.str() ) ;
  } // if
  else {
    ss << result.integer ;
    return new Token( INT, ss.str() ) ;
  } // else
} // Mult()

Token * Div( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  // cout << "is /" << endl ;
  bool isFloat = false ;
  OurNumber result ;

  result.integer = 0 ;
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "/" ) ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisNum = NULL ;
    try {
      thisNum = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisNum->mType != INT && thisNum->mType != FLOAT ) {
      if ( thisNum->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisNum, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "/", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "/", thisNum->mContent ) ;
      } // else
    } // if
    else {
      if ( !isFloat && thisNum->mType == FLOAT ) {
        isFloat = true ;
        result.real = ( float ) result.integer ;
      } // if

      if ( isFloat ) {
        float trueNum = atof( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.real = trueNum ;
        else {
          if ( trueNum == 0 )
            throw OurException( DIVZRO, "/" ) ;
          result.real /= trueNum ;
        } // else
      } // if
      else {
        int trueNum = atoi( thisNum->mContent.c_str() ) ;
        if ( i == 0 )
          result.integer = trueNum ;
        else {
          if ( trueNum == 0 )
            throw OurException( DIVZRO, "/" ) ;
          result.integer /= trueNum ;
        } // else
      } // else
    } // else
  } // for

  stringstream ss ;
  if ( isFloat ) {
    ss << fixed << setprecision( 3 ) << result.real ;
    return new Token( FLOAT, ss.str() ) ;
  } // if
  else {
    ss << result.integer ;
    return new Token( INT, ss.str() ) ;
  } // else
} // Div()

Token * Not( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "not" ) ;

  Token * t = NULL ;
  try {
    t = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( t->mType != NIL )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // Not()

Token * And( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "and" ) ;

  Token * t = NULL ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    try {
      t = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDCOND, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( t->mType == NIL )
      return new Token( NIL, "nil" ) ;
  } // for

  return t ;
} // And()

Token * Or( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "or" ) ;

  Token * t = NULL ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    try {
      t = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDCOND, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( t->mType != NIL )
      return t ;
  } // for

  return new Token( NIL, "nil" ) ;
} // Or()

Token * Bigg( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, ">" ) ;

  float a = 0, b = 0 ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != INT && thisArg->mType != FLOAT ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, ">", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, ">", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = atof( thisArg->mContent.c_str() ) ;
    else {
      b = atof( thisArg->mContent.c_str() ) ;
      if ( a <= b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Bigg()

Token * Bigeq( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, ">=" ) ;

  float a = 0, b = 0 ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != INT && thisArg->mType != FLOAT ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, ">=", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, ">=", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
    else {
      b = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
      if ( a < b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Bigeq()

Token * Sml( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "<" ) ;

  float a = 0, b = 0 ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != INT && thisArg->mType != FLOAT ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "<", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "<", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
    else {
      b = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
      if ( a >= b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Sml()

Token * Smleq( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "<=" ) ;

  float a = 0, b = 0 ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != INT && thisArg->mType != FLOAT ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "<=", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "<=", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
    else {
      b = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
      if ( a > b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Smleq()

Token * Eq( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "=" ) ;

  float a = 0, b = 0 ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != INT && thisArg->mType != FLOAT ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "=", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "=", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
    else {
      b = atof( EvalSExp( argList.at( i ), level + 1, localDefMap )->mContent.c_str() ) ;
      if ( a != b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Eq()

Token * Strapp( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  string resultStr ;
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "string-append" ) ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    Token * thisStr = NULL ;
    try {
      thisStr = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisStr->mType != STRING ) {
      if ( thisStr->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisStr, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "string-append", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "string-append", thisStr->mContent ) ;
      } // else
    } // if
    else {
      if ( resultStr == "" )
        resultStr = thisStr->mContent ;
      else {
        resultStr.erase( resultStr.end() - 1 ) ;
        string tempStr = thisStr->mContent ;
        tempStr.erase( tempStr.begin() ) ;
        resultStr += tempStr ;
      } // else
    } // else
  } // for

  return new Token( STRING, resultStr ) ;
} // Strapp()

Token * Strbig( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "string>?" ) ;

  string a = "", b = "" ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != STRING ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "string>?", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "string>?", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = thisArg->mContent.c_str() ;
    else {
      b = thisArg->mContent.c_str() ;
      if ( a <= b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Strbig()

Token * Strsml( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "string<?" ) ;

  string a = "", b = "" ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != STRING ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "string<?", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "string<?", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = thisArg->mContent.c_str() ;
    else {
      b = thisArg->mContent.c_str() ;
      if ( a >= b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Strsml()

Token * Streql( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 )
    throw OurException( WRONGARGNUM, "string=?" ) ;

  string a = "", b = "" ;
  bool result = true ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {

    Token * thisArg = NULL ;
    try {
      thisArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    if ( thisArg->mType != STRING ) {
      if ( thisArg->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( thisArg, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "string=?", ss.str() ) ;
      } // if
      else {
        cout << "" ;
        throw OurException( WRONGTYPE, "string=?", thisArg->mContent ) ;
      } // else
    } // if

    if ( i == 0 )
      a = thisArg->mContent.c_str() ;
    else {
      b = thisArg->mContent.c_str() ;
      if ( a != b )
        result = false ;
      else
        a = b ;
    } // else
  } // for

  if ( result )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // Streql()

Token * IsEqv( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 2 )
    throw OurException( WRONGARGNUM, "eqv?" ) ;

  Token * a = NULL ;
  Token * b = NULL ;
  int cPointer = 0 ;
  try {
    a = EvalSExp( argList.at( cPointer ), level + 1, localDefMap ) ;
    ++ cPointer ;
    b = EvalSExp( argList.at( cPointer ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( cPointer ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( a == b )
    return new Token( T, "#t" ) ;
  else {
    if ( ( a->mType == INT || a->mType == FLOAT ) &&
         ( b->mType == INT || b->mType == FLOAT ) ) {
      if ( a->mContent == b->mContent )
        return new Token( T, "#t" ) ;
      else
        return new Token( NIL, "nil" ) ;
    } // if
    else if ( a->mType == T && b->mType == T )
      return new Token( T, "#t" ) ;
    else if ( a->mType == NIL && b->mType == NIL )
      return new Token( T, "#t" ) ;
    else
      return new Token( NIL, "nil" ) ;
  } // else
} // IsEqv()

Token * IsEql( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 2 )
    throw OurException( WRONGARGNUM, "equal?" ) ;

  Token * a = NULL ;
  Token * b = NULL ;
  int cPointer = 0 ;
  try {
    a = EvalSExp( argList.at( cPointer ), level + 1, localDefMap ) ;
    ++ cPointer ;
    b = EvalSExp( argList.at( cPointer ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( cPointer ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  string data_a, data_b ;
  stringstream ss ;
  PrintSExp( a, 1, true, false, ss ) ;
  data_a = ss.str() ;
  ss.str( "" ) ;
  PrintSExp( b, 1, true, false, ss ) ;
  data_b = ss.str() ;

  if ( data_a == data_b )
    return new Token( T, "#t" ) ;
  else
    return new Token( NIL, "nil" ) ;
} // IsEql()

Token * Begin( int level, vector<Token*> & argList, TokenPtr root, map<string,TokenPtr> * localDefMap ) {
  if ( argList.empty() )
    throw OurException( WRONGARGNUM, "begin" ) ;

  Token * result = NULL ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    try {
      result = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( i == argList.size() - 1 && e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( i ), 1, true, true, ss ) ;
        throw OurException( NORET, ss.str() ) ;
      } // if
      else {
        if ( e.mEType != NORET )
          throw ;
      } // else
    } // catch()
  } // for

  return result ;
} // Begin()

Token * If( int level, vector<Token*> & argList, TokenPtr root, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 2 && argList.size() != 3 )
    throw OurException( WRONGARGNUM, "if" ) ;

  Token * condition = NULL ;
  try {
    condition = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDTEST, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( argList.size() == 2 ) {
    if ( condition->mType != NIL )
      return EvalSExp( argList.at( 1 ), level + 1, localDefMap ) ;
    else {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( NORET, ss.str() ) ;
    } // else
  } // if
  else {
    if ( condition->mType != NIL )
      return EvalSExp( argList.at( 1 ), level + 1, localDefMap ) ;
    else
      return EvalSExp( argList.at( 2 ), level + 1, localDefMap ) ;
  } // else
} // If()

Token * Cond( int level, vector<Token*> & argList, TokenPtr root, map<string,TokenPtr> * localDefMap ) {
  if ( argList.empty() ) {
    stringstream ss ;
    PrintSExp( root, 1, true, true, ss ) ;
    throw OurException( CNDERR, ss.str() ) ;
  } // if

  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    Token * condActSet = argList.at( i ) ;
    if ( condActSet->mType == SYMBOL && CheckDefine( condActSet ) )
      condActSet = EvalSExp( condActSet, level + 1, localDefMap ) ;

    if ( condActSet->mType != LEFT_PAREN ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( CNDERR, ss.str() ) ;
    } // if
    else {
      vector<Token*> actList ;
      CheckAndGetListElement( condActSet, actList ) ;
      if ( actList.empty() ) {
        stringstream ss ;
        PrintSExp( root, 1, true, true, ss ) ;
        throw OurException( CNDERR, ss.str() ) ;
      } // if
    } // else
  } // for

  Token * result = NULL ;
  for ( int i = 0 ; i < argList.size() ; ++ i ) {
    Token * condActSet = argList.at( i ) ;

    if ( condActSet->mType != LEFT_PAREN ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( CNDERR, ss.str() ) ;
    } // if

    Token * condition = NULL ;
    if ( i == argList.size() - 1 && condActSet->mPtrList[ LEFT ]->mContent == "else" )
      condition = new Token( T, "#t" ) ;
    else {
      try {
        condition = EvalSExp( condActSet->mPtrList[ LEFT ], level + 1, localDefMap ) ;
      } catch( OurException e ) {
        if ( e.mEType == NORET ) {
          stringstream ss ;
          PrintSExp( condActSet->mPtrList[ LEFT ], 1, true, true, ss ) ;
          throw OurException( UNBONDTEST, ss.str() ) ;
        } // if
        else
          throw ;
      } // catch()
    } // else

    Token * action = NULL ;
    if ( condActSet->mPtrList[ RIGHT ]->mPtrList[ LEFT ] == NULL ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( CNDERR, ss.str() ) ;
    } // if
    else {
      vector<Token*> actList ;
      CheckAndGetListElement( condActSet, actList ) ;

      if ( condition->mType != NIL && result == NULL ) {
        for ( int j = 0 ; j < actList.size() ; ++ j ) {
          try {
            action = EvalSExp( actList.at( j ), level + 1, localDefMap ) ;
          } catch ( OurException e ) {
            if ( j == actList.size() - 1 && e.mEType == NORET ) {
              stringstream ss ;
              PrintSExp( actList.at( j ), 1, true, true, ss ) ;
              throw OurException( NORET, ss.str() ) ;
            } // if
            else {
              if ( e.mEType != NORET )
                throw ;
            } // else
          } // catch()
        } // for

        return action ;
      } // if
    } // else
  } // for

  if ( result == NULL ) {
    stringstream ss ;
    PrintSExp( root, 1, true, true, ss ) ;
    throw OurException( NORET, ss.str() ) ;
  } // if

  return NULL ;
} // Cond()

Token * Let( int level, vector<Token*> & argList, TokenPtr root, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 ) {
    stringstream ss ;
    PrintSExp( root, 1, true, true, ss ) ;
    throw OurException( LETERR, ss.str() ) ;
  } // if
  else {
    Token * localDefHead = argList.at( 0 ) ;
    map<string,TokenPtr> * tempLocal = new map<string,TokenPtr>() ;
    map<string,TokenPtr> & tempLocalRef = *tempLocal ;

    if ( localDefMap != NULL ) {
      for ( map<string,TokenPtr>::iterator it = localDefMap->begin() ;
            it != localDefMap->end() ; ++ it ) {
        tempLocalRef[ it->first ] = it->second ;
      } // for
    } // if

    if ( localDefHead->mType != LEFT_PAREN && localDefHead->mType != NIL ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( LETERR, ss.str() ) ;
    } // if
    else {
      vector<TokenPtr> localDefList ;
      CheckAndGetPairElement( localDefHead, localDefList ) ;
      // We will check format by this for loop
      for ( int i = 0 ; i < localDefList.size() ; ++ i ) {
        if ( localDefList.at( i )->mType != LEFT_PAREN ) {
          stringstream ss ;
          PrintSExp( root, 1, true, true, ss ) ;
          throw OurException( LETERR, ss.str() ) ;
        } // if
        else {
          vector<TokenPtr> localDefSet ;
          CheckAndGetPairElement( localDefList.at( i ), localDefSet ) ;
          if ( localDefSet.size() == 2 ) {
            if ( localDefSet.at( 0 )->mType != SYMBOL || IsCallable( localDefSet.at( 0 ) ) ) {
              stringstream ss ;
              PrintSExp( root, 1, true, true, ss ) ;
              throw OurException( LETERR, ss.str() ) ;
            } // if
          } // if
          else {
            stringstream ss ;
            PrintSExp( root, 1, true, true, ss ) ;
            throw OurException( LETERR, ss.str() ) ;
          } // else
        } // else
      } // for

      for ( int i = 0 ; i < localDefList.size() ; ++ i ) {
        vector<TokenPtr> localDefSet ;
        CheckAndGetPairElement( localDefList.at( i ), localDefSet ) ;
        // WATCHOUT!! if you set a value to map obj by giving EvalSExp()'s
        // return value directly, when EvalSExp() failed there is no return value
        // return to map obj, AN EMPTY MAP VALUE WILL BE CREATED.
        Token * evalResult = NULL ;
        try {
          evalResult = EvalSExp( localDefSet.at( 1 ), level + 1, localDefMap ) ;
        } catch( OurException e ) {
          if ( e.mEType == NORET ) {
            stringstream ss ;
            PrintSExp( localDefSet.at( 1 ), 1, true, true, ss ) ;
            throw OurException( NORET_B, ss.str() ) ;
          } // if
          else
            throw ;
        } // catch()
        // Here, we use a temp value named evalResult to save EvalSExp()'s result
        // If EvalSExp() failed, no map obj's key-value set are created.
        // It means THERE IS NO BINDING ACTION execute.
        tempLocalRef[ localDefSet.at( 0 )->mContent ] = evalResult ;
        // There is a question here:
        // Do we need send this level's local define map to next level's EvalSExp()?
      }  // for

      // And then, let us evaluate all action and return the last action's value back
      Token * result = NULL ;
      for ( int i = 1 ; i < argList.size() ; ++ i ) {
        try {
          if ( argList.at( i )->mType == LEFT_PAREN
               && argList.at( i )->mPtrList[ LEFT ]->mContent != "eval" )
            result = EvalSExp( argList.at( i ), level + 1, tempLocal ) ;
          else
            result = EvalSExp( argList.at( i ), level, tempLocal ) ;
        } catch( OurException e ) {
          if ( i == argList.size() - 1 && e.mEType == NORET ) {
            stringstream ss ;
            PrintSExp( argList.at( i ), 1, true, true, ss ) ;
            throw OurException( NORET, ss.str() ) ;
          } // if
          else {
            if ( e.mEType != NORET )
              throw ;
          } // else
        } // catch()
      } // for

      return result ;
    } // else
  } // else
} // Let()

Token * Lambda( int level, vector<Token*> & argList, TokenPtr root, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() < 2 ) {
    stringstream ss ;
    PrintSExp( root, 1, true, true, ss ) ;
    throw OurException( LAMBERR, ss.str() ) ;
  } // if
  else {
    Token * symbolListHead = argList.at( 0 ) ;
    if ( symbolListHead->mType != LEFT_PAREN && symbolListHead->mType != NIL ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( LAMBERR, ss.str() ) ;
    } // if

    // Create a Ourfunction object and give it a name.
    TokenPtr funcNode = new Token( USRFUNC, "#<procedure lambda>" ) ;
    funcNode->mFunc = new OurFunction( "lambda" ) ; // lambda function

    // Get all symbol we need to bind for a function
    vector<TokenPtr> symbolList ;
    CheckAndGetPairElement( symbolListHead, symbolList ) ;
    for ( int i = 0 ; i < symbolList.size() ; ++ i ) {
      if ( symbolList.at( i )->mType == SYMBOL && ( !IsCallable( symbolList.at( i ) )
                                                    || symbolList.at( i )->mType == USRFUNC ) )
        funcNode->mFunc->SetArg( symbolList.at( i )->mContent ) ;
      else {
        stringstream ss ;
        PrintSExp( root, 1, true, true, ss ) ;
        throw OurException( LAMBERR, ss.str() ) ;
      } // else
    } // for

    // Set all action to OurFunction object
    for ( int i = 1 ; i < argList.size() ; ++ i ) {
      // TokenPtr evalArg = EvalSExp( argList.at( i ), level + 1, localDefMap ) ;
      // funcNode->mFunc->SetFunction( evalArg ) ;
      funcNode->mFunc->SetFunction( argList.at( i ) ) ;
    } // for

    return funcNode ;
  } // else

  return new Token( STRING, "is lambda" ) ;
} // Lambda()

Token * Usrfunc( int level, vector<Token*> & argList, TokenPtr thisOperator,
                 TokenPtr root, map<string,TokenPtr> * localDefMap ) {
  TokenPtr result  = NULL ;
  try {
    result = thisOperator->mFunc->Evaluate( argList, level, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( NORET, ss.str() ) ;
    } // if
    else {
      throw ;
    } // else
  } // catch()

  return result ;
} // Usrfunc()

Token * Eval( vector<Token*> & argList, int level, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "eval" ) ;
  else {
    TokenPtr targetArg = NULL ;
    try {
      targetArg = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
        throw OurException( NORET, ss.str() ) ;
      } // if
      else {
        throw ;
      } // else
    } // catch()

    return EvalSExp( targetArg, 0, localDefMap ) ;
  } // else
} // Eval()

Token * DisplayString( vector<Token*> & argList, int level, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "display-string" ) ;
  else {
    TokenPtr targetString = NULL ;

    try {
      targetString = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
    } catch ( OurException e ) {
      if ( e.mEType == NORET ) {
        stringstream ss ;
        PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
        throw OurException( UNBONDPARA, ss.str() ) ;
      } // if
      else
        throw ;
    } // catch()

    int t = targetString->mType ;
    // cout << targetString->mContent << " " << t << endl ;
    if ( t != STRING && t != ERROBJ ) {
      if ( targetString->mType == LEFT_PAREN ) {
        stringstream ss ;
        PrintSExp( targetString, 1, true, true, ss ) ;
        throw OurException( WRONGTYPE, "display-string", ss.str() ) ;
      } // if
      else {
        int i = 0 ;
        throw OurException( WRONGTYPE, "display-string", targetString->mContent ) ;
      } // else
    } // if
    else {
      string str = targetString->mContent ;
      int charPos = str.find_first_of( '\"' ) ;
      if ( charPos != -1 ) {
        str.erase( str.begin() + charPos ) ;
        charPos = str.find_last_of( '\"' ) ;
        if ( charPos != -1 )
          str.erase( str.begin() + charPos ) ;
      } // if

      cout << str ;
      return targetString ;
    } // else
  } // else
} // DisplayString()

Token * Read( vector<Token*> & argList ) {
  if ( !argList.empty() )
    throw OurException( WRONGARGNUM, "read" ) ;
  TokenPtr thisToken = NULL ;

  gLine = 1 ;
  gCol = 0 ;
  try {
    gSc.mAcceptType[ DOT ] = false ;
    gSc.mAcceptType[ RIGHT_PAREN ] = false ;
    thisToken = ReadToken() ;
    if ( thisToken->mType == LEFT_PAREN || thisToken->mType == QUOTE ) {
      gSc.mHasOutput = false ;
      ReadSExp( 0, thisToken ) ;
      DebugPrint( thisToken ) ;
    } // if
  } catch ( OurException e ) {
    stringstream ss ;
    ss << "\"" << e.mErrMsg << "\"" ;
    thisToken = new Token( ERROBJ, ss.str() ) ;
  } // catch()

  return thisToken ;
} // Read()

Token * Write( vector<Token*> & argList, int level, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "write" ) ;
  TokenPtr thisExp = NULL ;
  try {
    thisExp = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  stringstream ss ;
  PrintSExp( thisExp, 1, true, false, ss ) ;
  cout << ss.str() ;
  return thisExp ;
} // Write()

Token * Newline( vector<Token*> & argList ) {
  if ( !argList.empty() )
    throw OurException( WRONGARGNUM, "newline" ) ;
  cout << endl ;
  return new Token( NIL, "nil" ) ;
} // Newline()

Token * Symbol2string( vector<Token*> & argList, int level, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "symbol->string" ) ;
  TokenPtr thisSym = NULL ;
  try {
    thisSym = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  // cout << "[" << thisSym->mContent << "]" << endl ;
  if ( thisSym->mType != SYMBOL && !IsCallable( thisSym ) ) {
    if ( thisSym->mType == LEFT_PAREN ) {
      stringstream ss ;
      PrintSExp( thisSym, 1, true, true, ss ) ;
      throw OurException( WRONGTYPE, "symbol->string", ss.str() ) ;
    } // if
    else {
      int i = 0 ;
      throw OurException( WRONGTYPE, "symbol->string", thisSym->mContent ) ;
    } // else
  } // if
  else {
    stringstream ss ;
    ss << '\"' << thisSym->mContent << '\"' ;
    return new Token( STRING, ss.str() ) ;
  } // else

  return NULL ;
} // Symbol2string()

Token * Number2string( vector<Token*> & argList, int level, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "symbol->string" ) ;
  TokenPtr thisNum = NULL ;
  try {
    thisNum = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( thisNum->mType != INT && thisNum->mType != FLOAT ) {
    if ( thisNum->mType == LEFT_PAREN ) {
      stringstream ss ;
      PrintSExp( thisNum, 1, true, true, ss ) ;
      throw OurException( WRONGTYPE, "number->string", ss.str() ) ;
    } // if
    else {
      int i = 0 ;
      throw OurException( WRONGTYPE, "number->string", thisNum->mContent ) ;
    } // else
  } // if
  else {
    stringstream ss ;
    ss << '\"' << thisNum->mContent << '\"' ;
    return new Token( STRING, ss.str() ) ;
  } // else

  return NULL ;
} // Number2string()

Token * Set( int level, Token * root, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 2 ) {
    stringstream ss ;
    PrintSExp( root, 1, true, true, ss ) ;
    throw OurException( SETERR, ss.str() ) ;
  } // if
  else {
    Token * symNeedDef = argList.at( 0 ) ;
    if ( symNeedDef->mType != SYMBOL || IsCallable( symNeedDef ) ) {
      stringstream ss ;
      PrintSExp( root, 1, true, true, ss ) ;
      throw OurException( SETERR, ss.str() ) ;
    } // if
    else {
      if ( argList.at( 1 )->mType == SYMBOL && ( !CheckDefine( argList.at( 1 ) )
                                                 && !IsCallable( argList.at( 1 ) ) ) ) {
        if ( localDefMap->find( argList.at( 1 )->mContent ) == localDefMap->end() )
          throw OurException( UNBOND, argList.at( 1 )->mContent ) ;
      } // if

      Token * defResult = NULL ;
      try {
        defResult = EvalSExp( argList.at( 1 ), level + 1, localDefMap ) ;
      } catch ( OurException e ) {
        if ( e.mEType == NORET ) {
          stringstream ss ;
          PrintSExp( argList.at( 1 ), 1, true, true, ss ) ;
          throw OurException( NORET_B, ss.str() ) ;
        } // if
        else
          throw ;
      } // catch()

      if ( defResult->mType == USRFUNC && defResult->mContent == "#<procedure lambda>" )
        defResult->mFunc->SetName( "lambda" ) ;

      if ( localDefMap != NULL && localDefMap->find( symNeedDef->mContent ) != localDefMap->end() ) {
        map<string,TokenPtr> & localDefMapRef = *localDefMap ;
        localDefMapRef[ symNeedDef->mContent ] = defResult ;
      } // if
      else
        gDefineMap[ symNeedDef->mContent ] = defResult ;

      return defResult ; // OOO defined.
    } // else
  } // else
} // Set()

Token * CreateErrorObj( vector<Token*> & argList, int level, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "create-error-object" ) ;
  TokenPtr thisStr = NULL ;
  try {
    thisStr = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( thisStr->mType != STRING ) {
    if ( thisStr->mType == LEFT_PAREN ) {
      stringstream ss ;
      PrintSExp( thisStr, 1, true, true, ss ) ;
      throw OurException( WRONGTYPE, "create-error-object", ss.str() ) ;
    } // if
    else {
      int i = 0 ;
      throw OurException( WRONGTYPE, "create-error-object", thisStr->mContent ) ;
    } // else
  } // if

  return new Token( ERROBJ, thisStr->mContent ) ;
} // CreateErrorObj()

Token * IsErrobj( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "error-object?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != ERROBJ )
    return new Token( NIL, "nil" ) ;
  else
    return new Token( T, "#t" ) ;
} // IsErrobj()

Token * Verbos( int level, vector<Token*> & argList, map<string,TokenPtr> * localDefMap ) {
  if ( argList.size() != 1 )
    throw OurException( WRONGARGNUM, "error-object?" ) ;

  Token * pairRoot = NULL ;
  try {
    pairRoot = EvalSExp( argList.at( 0 ), level + 1, localDefMap ) ;
  } catch ( OurException e ) {
    if ( e.mEType == NORET ) {
      stringstream ss ;
      PrintSExp( argList.at( 0 ), 1, true, true, ss ) ;
      throw OurException( UNBONDPARA, ss.str() ) ;
    } // if
    else
      throw ;
  } // catch()

  if ( pairRoot->mType != NIL ) {
    gVerbos = true ;
    return new Token( T, "#t" ) ;
  } // if
  else {
    gVerbos = false ;
    return new Token( NIL, "nil" ) ;
  } // else
} // IsErrobj()

Token * IsVerbos( vector<Token*> & argList ) {
  if ( !argList.empty() )
    throw OurException( WRONGARGNUM, "verbos?" ) ;

  if ( gVerbos ) {
    return new Token( T, "#t" ) ;
  } // if
  else {
    return new Token( NIL, "nil" ) ;
  } // else
} // IsErrobj()

Token * EvalSExp( Token * root, int level, map<string,TokenPtr> * localDefMap ) {
  // cout << level << endl ;
  if ( root->mType == LEFT_PAREN ) {
    if ( root->mContent == "(" ) {
      vector<Token*> argList ;
      CheckAndGetListElement( root, argList ) ;

      Token * thisOperator = NULL ;
      // A SExp's function has four type: normal_internal, another_SExp, defined_internal, non_sense
      if ( root->mPtrList[ LEFT ]->mType == LEFT_PAREN ) {
        try {
          thisOperator = EvalSExp( root->mPtrList[ LEFT ],
                                          level + 1, localDefMap ) ; // ( ( ... ) ... )
        } catch( OurException e ) {
          if ( e.mEType == NORET ) {
            stringstream ss ;
            PrintSExp( root->mPtrList[ LEFT ], 1, true, true, ss ) ;
            throw OurException( NORET_B, ss.str() ) ;
          } // if
          else
            throw ;
        } // catch()
      } // if
      else
        thisOperator = EvalSExp( root->mPtrList[ LEFT ], level, localDefMap ) ;

      if ( IsCallable( thisOperator ) ) {
        // type normal_internal an type 2 and 3 will be processed here
        try {
          if ( thisOperator->mType == CONS )
            return Cons( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == LIST )
            return List( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == QUOTE ) {
            // stringstream ss ;
            // PrintSExp( root->mPtrList[ RIGHT ]->mPtrList[ LEFT ], 1, true, ss ) ;
            // cout << "is quote!" << endl ;
            if ( argList.size() != 1 )
              throw OurException( WRONGARGNUM, "quote" ) ;

            Quote( root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] ) ;
            return root->mPtrList[ RIGHT ]->mPtrList[ LEFT ] ;
          } // else if
          else if ( thisOperator->mType == DEFINE )
            return Define( level, root, argList, localDefMap ) ;
          else if ( thisOperator->mType == CAR )
            return Car( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == CDR )
            return Cdr( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISPAIR )
            return IsPair( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISLIST )
            return IsList( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISATOM )
            return IsAtom( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISNULL )
            return IsNull( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISINT )
            return IsInt( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISREAL )
            return IsReal( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISNUM )
            return IsNum( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISSTR )
            return IsStr( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISBOOL )
            return IsBool( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISSYM )
            return IsSym( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ADD )
            return Add( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == SUB )
            return Sub( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == MULT )
            return Mult( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == DIV )
            return Div( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == NOT )
            return Not( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == AND )
            return And( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == OR )
            return Or( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == BIGG )
            return Bigg( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == BIGEQ )
            return Bigeq( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == SML )
            return Sml( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == SMLEQ )
            return Smleq( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == EQ )
            return Eq( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == STRAPP )
            return Strapp( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == STRBIG )
            return Strbig( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == STRSML )
            return Strsml( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == STREQL )
            return Streql( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISEQV )
            return IsEqv( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISEQL )
            return IsEql( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == BEGIN )
            return Begin( level, argList, root, localDefMap ) ;
          else if ( thisOperator->mType == IF )
            return If( level, argList, root, localDefMap ) ;
          else if ( thisOperator->mType == COND )
            return Cond( level, argList, root, localDefMap ) ;
          else if ( thisOperator->mType == LET )
            return Let( level, argList, root, localDefMap ) ;
          else if ( thisOperator->mType == LAMBDA )
            return Lambda( level, argList, root, localDefMap ) ;
          else if ( thisOperator->mType == USRFUNC )
            return Usrfunc( level, argList, thisOperator, root, localDefMap ) ;
          else if ( thisOperator->mType == EXIT ) {
            // cout << "is exit" << endl ;
            if ( level > 0 )
              throw OurException( EXITLVERR ) ;
            else if ( !argList.empty() )
              throw OurException( WRONGARGNUM, "exit" ) ;

            gProgEnd = true ;
            return NULL ;
          } // else if
          else if ( thisOperator->mType == CLEAN ) {
            // cout << "is clean-enviroment" << endl ;
            if ( level > 0 )
              throw OurException( CLEANLVERR ) ;
            else if ( !argList.empty() )
              throw OurException( WRONGARGNUM, "clean-environment" ) ;

            gDefineMap.clear() ;

            for ( int i = 0 ; i < cmdNum ; ++ i ) {
              stringstream ss ;
              ss << "#<procedure " << gInternalCmd[ i ] << ">" ;
              gDefineMap[ gInternalCmd[ i ] ] = new Token( i + 12, ss.str() ) ;
            } // for

            if ( gVerbos )
              cout << "environment cleaned" << endl ;
            return NULL ;
          } // else if
          else if ( thisOperator->mType == PRINT ) {
            stringstream ss ;
            // PrintSExp( root, 1, true, true, ss ) ;
            for ( int i = 0 ; i < argList.size() ; ++ i )
              ss << argList.at( i )->mContent << " is " <<
                    EvalSExp( argList.at( i ), level, localDefMap )->mContent << endl ;
            cout << "{" << ss.str() << "}" << endl ;
            return NULL ;
          } // else if
          else if ( thisOperator->mType == READ )
            return Read( argList ) ;
          else if ( thisOperator->mType == WRITE )
            return Write( argList, level, localDefMap ) ;
          else if ( thisOperator->mType == EVAL )
            return Eval( argList, level, localDefMap ) ;
          else if ( thisOperator->mType == DSPSTR )
            return DisplayString( argList, level, localDefMap ) ;
          else if ( thisOperator->mType == NEWLINE ) {
            return Newline( argList ) ;
          } // else if
          else if ( thisOperator->mType == SYMTOSTR ) {
            return Symbol2string( argList, level, localDefMap ) ;
          } // else if
          else if ( thisOperator->mType == NUMTOSTR )
            return Number2string( argList, level, localDefMap ) ;
          else if ( thisOperator->mType == SET )
            return Set( level, root, argList, localDefMap ) ;
          else if ( thisOperator->mType == CEATEOBJ )
            return CreateErrorObj( argList, level, localDefMap ) ;
          else if ( thisOperator->mType == ISERROBJ )
            return IsErrobj( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == VERBOS )
            return Verbos( level, argList, localDefMap ) ;
          else if ( thisOperator->mType == ISVERBOS )
            return IsVerbos( argList ) ;
        } catch ( OurException e ) {
          if ( e.mEType == NORET && level == 0 ) {
            stringstream ss ;
            PrintSExp( root, 1, true, false, ss ) ;
            throw OurException( NORET, ss.str() ) ;
          } // if
          else
            throw ;
        } // catch()
      } // if
      else {
        // non_sense here
        if ( thisOperator->mType == SYMBOL && !thisOperator->mIsQuoted ) {
          throw OurException( UNBOND, thisOperator->mContent ) ;
        } // if
        else {
          stringstream ss ;
          PrintSExp( thisOperator, 1, true, false, ss ) ;
          throw OurException( NONFUNC, ss.str() ) ;
        } // else
      } // else
    } // if
  } // if
  else if ( root->mType == SYMBOL || root->mType == QUOTE ) {
    if ( localDefMap != NULL && localDefMap->find( root->mContent ) != localDefMap->end() ) {
      map<string,TokenPtr> & localDefMapRef = *localDefMap ;
      return localDefMapRef[ root->mContent ] ;
    } // if
    else if ( CheckDefine( root ) )
      return gDefineMap[ root->mContent ] ; // A defined symbol in, return its value out
    else if ( IsCallable( root ) ) {
      // stringstream ss ;
      // ss << "#<procedure " << root->mContent << ">" ;
      // root->mContent = ss.str() ;
      return root ;
    } // else if
    else
      throw OurException( UNBOND, root->mContent ) ;
  } // else if

  return root ;
} // EvalSExp()

int main() {
  /*
  Entry point here.
  */
  // scanf( "%*d%*c" ) ;
  cout << "Welcome to OurScheme!" << endl ;
  Token * thisToken = NULL ;


  for ( int i = 0 ; i < cmdNum ; ++ i ) {
    stringstream ss ;
    ss << "#<procedure " << gInternalCmd[ i ] << ">" ;
    gDefineMap[ gInternalCmd[ i ] ] = new Token( i + 12, ss.str() ) ;
    // cout << gDefineMap[ gInternalCmd[ i ] ]->mContent << endl ;
  } // for


  while ( !gProgEnd ) {
    cout << "\n> ";
    try {
      gSc.mAcceptType[ DOT ] = false ;
      gSc.mAcceptType[ RIGHT_PAREN ] = false ;
      thisToken = ReadToken() ;

      stringstream ss ;
      if ( thisToken->mType == LEFT_PAREN || thisToken->mType == QUOTE ) {
        gSc.mHasOutput = false ;
        ReadSExp( 0, thisToken ) ;
      } // if

      DebugPrint( thisToken ) ;

      thisToken = EvalSExp( thisToken, 0, NULL ) ;

      if ( thisToken != NULL ) {
        PrintSExp( thisToken, 1, true, false, ss ) ;
        cout << ss.str() << endl ;
      } // if

      gSc.mHasOutput = true ;
      gLine = 1 ;
      gCol = 0 ;
    } catch ( OurException e ) {
      printf( "%s", e.mErrMsg.c_str() );
      if ( !gProgEnd )
        printf( "\n" ) ;
      gSc.mHasOutput = false ;

      char c = '\0' ;
      string t ;
      do {
        c = getchar() ;
        t += c ;
      } while ( c != -1 && c != '\n' ) ;

      bool endBack = false, commentEnd = false ;
      for ( int i = 0 ; i < t.size() && !endBack ; ++ i ) {
        if ( t.at( i ) == ';' )
          commentEnd = true ;

        if ( !isspace( t.at( i ) ) && !commentEnd ) {
          for ( int j = 0 ; j < t.size() ; ++ j )
            cin.putback( t.at( t.size() - 1 - j ) ) ;
          endBack = true ;
        } // if
      } // for
    } // catch
  } // while

  printf( "\nThanks for using OurScheme!" );
  return 0;
} // main()
