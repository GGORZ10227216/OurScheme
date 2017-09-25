// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
/*TODO code clean*/
/*TODO fix other unexcept char error*/
#include <cctype>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <iomanip>

using namespace std;

bool gProgEnd = false;
int gLine = 1, gCol = 0;

enum tokenType {
  EMPTY, NIL, INT, FLOAT, DOT, T, QUOTE, STRING,
  SHARP, OPERATOR, LEFT_PAREN, RIGHT_PAREN, SYMBOL
};

enum excpType { NULLERR, EOFENCT, REQ_ATOM, REQ_RIGHT_P, EOLENCT };

enum tPtrType { LEFT, RIGHT, PRE } ;

class Token {
public:
  int mType;
  string content;
  Token * ptrList[ 3 ] = { NULL } ;
  Token() {
    mType = EMPTY;
    content = "";
  } // Token

  Token(int _type, string _content) {
    mType = _type;
    content = _content;
  } // Token
};

class Scanner;
class OurException : public exception {
public:
  int mEType;
  string mErrMsg;

  OurException() {
    mEType = NULL;
    mErrMsg = "Why do you want to throw an empty exception?";
  } // OurException()

  OurException( Token * _token, int _type, char deli = '\0' ) {
    stringstream ss;
    if (_type == REQ_ATOM) {
      if ( _token->content == ")" )
        ++ gCol ;
      ss << "ERROR (unexpected token) : "
         << "atom or '(' expected when token at Line "
         << gLine
         << " column "
         << gCol - _token->content.size()
         << " is >>" << _token->content << "<<" ;
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
    else if ( _type == REQ_RIGHT_P ) {
      ss << "ERROR (unexpected token) : "
         << "\')\' expected when token at Line "
         << gLine
         << " column "
         << gCol - _token->content.size() + 1
         << " is >>" << _token->content << "<<" ;
      char c = 0 ;
      do {
        c = getchar() ;
      } while (c != '\n' && c != -1 ) ;

      mErrMsg = ss.str();

      gLine = 1;
      gCol = 0;
    } // else if
  } // OurException()

  OurException(int _type) {
    if (_type == EOFENCT) {
      mErrMsg = "ERROR (no more input) : END-OF-FILE encountered";
      gProgEnd = true;
    } // if
    else if ( _type == EOLENCT ) {
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

  virtual ~OurException() throw() {}
};

class Scanner {
public:
  bool hasOutput = false;
  bool acceptType[ 13 ] ;
  friend class OurException;
  Scanner() {
    gLine = 1;
    gCol = 0;
    for ( int i = 0 ; i < 13 ; ++ i )
      acceptType[ i ] = true ;
  } // Scanner()

  char Getchar() /* throw ( OurException )*/ {
    char c = getchar();
    if (c == -1)
      return '\r';
    ++gCol;
    return c;
  } // Getchar()

  void Putback(char _ch) {
    cin.putback( _ch );
    --gCol;
  } // Putback()
private:
  string mLineBuf;
};

Scanner sc;
Token * ReadToken() {
  /*ReadToken() is a function witch can read a token
  *and determine it's type then return it to ReadSExp
  */

  /*DOT, OPERATOR and SHARP are mid-type, they are only use in ReadToken(),
  *and should not appear in other function.*/

  bool encounterTEnd = false, inComment = false;
  char buf = '\0';
  Token * resultToken = new Token();
  while (!encounterTEnd) {
    buf = sc.Getchar();
    if (resultToken->mType == EMPTY) {
      if (isalpha(buf)) {
        if (buf == 't')
          resultToken->mType = T;
        else
          resultToken->mType = SYMBOL;
      } // if
      else if (isdigit(buf)) {
        resultToken->mType = INT;
      } // else if
      else if (buf == '(' || buf == ')') {
        if (buf == '(')
          resultToken->mType = LEFT_PAREN;
        else
          resultToken->mType = RIGHT_PAREN;
        encounterTEnd = true;
      } // else if
      else if (buf == '.') {
        resultToken->mType = DOT;
      } // else if
      else if (buf == '\'') {
        resultToken->mType = QUOTE;
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        resultToken->mType = STRING;
      } // else if
      else if (buf == '#') {
        resultToken->mType = SHARP;
      } // else if
      else if (buf == '+' || buf == '-') {
        resultToken->mType = OPERATOR;
      } // else if
      else if (isspace(buf)) {
        if ( buf == '\n' ) {
          if ( sc.hasOutput ) {
            gLine = 1 ;
            sc.hasOutput = false ;
          } // if
          else
            ++gLine ;
          gCol = 0 ;
        } // if
        else if ( buf == '\r' )
          throw OurException(EOFENCT);

        // encounterTEnd = true ;
      } // else if
      else if (buf == ';') {
        char t = 0 ;
        do {
          t = sc.Getchar() ;
          if ( t == '\r' )
            throw OurException( EOFENCT ) ;
        } while ( t != '\n' ) ;

        sc.Putback( '\n' );
      } // else if
      else
        resultToken->mType = SYMBOL;

      if (!isspace(buf) && buf != ';')
        resultToken->content += buf;
    } // if
    else if (resultToken->mType == SHARP) {
      if (isalpha(buf)) {
        if (buf == 'f')
          resultToken->mType = NIL;
        else if (buf == 't')
          resultToken->mType = T;
        else
          resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '(' || buf == ')') {
        sc.Putback(buf);
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if (buf == '.') {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '\'') {
        sc.Putback(buf);
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        sc.Putback(buf);
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if (isspace(buf)) {
        resultToken->mType = SYMBOL;
        sc.Putback( buf ) ;
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == OPERATOR) {
      if (isalpha(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->mType = INT;
        resultToken->content += buf;
      } // else if
      else if (buf == '(' || buf == ')') {
        sc.Putback(buf);
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if (buf == '.') {
        resultToken->mType = FLOAT;
        resultToken->content += buf;
      } // else if
      else if (buf == '\'') {
        sc.Putback(buf);
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        sc.Putback(buf);
        resultToken->mType = SYMBOL;
        encounterTEnd = true;
      } // else if
      else if (isspace(buf)) {
        resultToken->mType = SYMBOL;
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == NIL) {
      if (isalpha(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '.') {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '\"' || isspace(buf) || buf == '\'' ||
               buf == '(' || buf == ')' ) {
        // if ( !sc.acceptType[ NIL ] )
        //     throw OurException( buf, UNEXCEPT );
        // else {
        sc.Putback(buf);
        encounterTEnd = true;
        // } // else
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == DOT) {
      if (isalpha(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->mType = FLOAT;
        resultToken->content += buf;
      } // else if
      else if (buf == '.') {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if ( buf == '\'' || buf == '\"' || isspace(buf) ||
                buf == '(' || buf == ')' ) {
        if ( !sc.acceptType[ DOT ] )
          throw OurException( resultToken, REQ_ATOM, buf );
        else {
          sc.Putback(buf);
          encounterTEnd = true;
        } // else
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == INT) {
      if (isalpha(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->content += buf;
      } // else if
      else if (buf == '(' || buf == ')') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '.') {
        resultToken->mType = FLOAT;
        resultToken->content += buf;
      } // else if
      else if (buf == '\'') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (isspace(buf)) {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == FLOAT) {
      if (isalpha(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->content += buf;
      } // else if
      else if (buf == '(' || buf == ')') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '.') {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '\'') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (isspace(buf)) {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == T) {
      if (isalpha(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // if
      else if (isdigit(buf)) {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '(' || buf == ')') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '.') {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else if
      else if (buf == '\'') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (isspace(buf)) {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else {
        resultToken->mType = SYMBOL;
        resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == STRING) {
      if ( buf != '\"' ) {
        bool fakeEsc = false ;
        char escChar = '\0' ;
        if (buf == '\n')
          throw OurException( EOLENCT );
        else if ( buf == '\r' ) {
          throw OurException( EOFENCT ) ;
        } // else if
        else if ( buf == '\\' ) {
          escChar = sc.Getchar() ;
          if ( escChar == 'n' )
            buf = '\n' ;
          else if ( escChar == 't' )
            buf = '\t' ;
          else if ( escChar == '\"' )
            buf = '\"' ;
          else if ( escChar != '\\' )
            fakeEsc = true ;
        } // else if

        resultToken->content += buf;
        if ( fakeEsc )
          resultToken->content += escChar ;
      } // if
      else {
        encounterTEnd = true;
        if (buf == '\"')
          resultToken->content += buf;
      } // else
    } // else if
    else if (resultToken->mType == SYMBOL) {
      if (buf == '(' || buf == ')') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '\'') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (buf == '\"') {
        sc.Putback(buf);
        encounterTEnd = true;
      } // else if
      else if (!isspace(buf)) {
        resultToken->content += buf;
      } // if
      else {
        encounterTEnd = true;
        sc.Putback(buf);
      } // else
    } // else if
  } // while

  if ( !sc.acceptType[ resultToken->mType ] )
    throw OurException( resultToken, REQ_ATOM, buf ) ;

  return resultToken;
} // ReadToken()

void ReadSExp( int level, Token * root ) {
  Token * thisNode = NULL ;
  bool hasRightParen = true ;
  if ( root->mType == QUOTE ) {
    root->mType = LEFT_PAREN ;
    root->content = "(" ;
    root->ptrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
    root->ptrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
    sc.acceptType[ DOT ] = false ; // ( '.
    sc.acceptType[ RIGHT_PAREN ] = false ; // ( ')
    root->ptrList[ RIGHT ]->ptrList[ LEFT ] = ReadToken() ;
    if ( root->ptrList[ RIGHT ]->ptrList[ LEFT ]->mType == LEFT_PAREN ) // ( ( quote . ( ( ...
      ReadSExp( level + 1, root->ptrList[ RIGHT ]->ptrList[ LEFT ] ) ;
    else if ( root->ptrList[ RIGHT ]->ptrList[ LEFT ]->mType == QUOTE ) {
      ReadSExp( level + 1, root->ptrList[ RIGHT ]->ptrList[ LEFT ] ) ;
    } // else if

    hasRightParen = false ;
  } // if

  if ( root->ptrList[ LEFT ] == NULL ) {
    // (
    sc.acceptType[ DOT ] = false ; // ( '.
    sc.acceptType[ RIGHT_PAREN ] = true ; // ()
    root->ptrList[ LEFT ] = ReadToken() ;
    // cout << root->ptrList[ LEFT ]->content << endl ;
    if ( root->ptrList[ LEFT ]->mType == LEFT_PAREN ) // ( ( ...
      ReadSExp( level + 1, root->ptrList[ LEFT ] ) ;
    else if ( root->ptrList[ LEFT ]->mType == QUOTE ) { // ( '
      root->ptrList[ LEFT ] = new Token( LEFT_PAREN, "(" ) ;
      root->ptrList[ LEFT ]->ptrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
      root->ptrList[ LEFT ]->ptrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      thisNode = root->ptrList[ LEFT ]->ptrList[ RIGHT ] ;
      // ( ( quote . (
      sc.acceptType[ DOT ] = false ; // ( '.
      sc.acceptType[ RIGHT_PAREN ] = false ; // ( ')
      thisNode->ptrList[ LEFT ] = ReadToken() ; // ( ( quote . ( a . nil )
      if ( thisNode->ptrList[ LEFT ]->mType == LEFT_PAREN ) // ( ( quote . ( ( ...
        ReadSExp( level + 1, thisNode->ptrList[ LEFT ] ) ;
      else if ( thisNode->ptrList[ LEFT ]->mType == QUOTE ) {
        ReadSExp( level + 1, thisNode->ptrList[ LEFT ] ) ;
      } // else if
    } // else if
    else if ( root->ptrList[ LEFT ]->mType == RIGHT_PAREN ) {
      // ()
      root->mType = NIL ;
      root->content = "nil" ;
      root->ptrList[LEFT] = NULL ;
      hasRightParen = false ;
      return ;
    } // else if
  } // if
  if ( root->ptrList[ RIGHT ] == NULL ) {
    // ( a
    sc.acceptType[ DOT ] = true ; // ( a .
    sc.acceptType[ RIGHT_PAREN ] = true ; // ( a )
    root->ptrList[ RIGHT ] = ReadToken() ;
    // cout << "LEFT " << root->ptrList[ RIGHT ]->content << endl ;
    if ( root->ptrList[ RIGHT ]->mType == DOT ) {
      // ( a .
      sc.acceptType[ DOT ] = false ; // ( '.
      sc.acceptType[ RIGHT_PAREN ] = false ; // ( ')
      root->ptrList[ RIGHT ] = ReadToken() ;
      // cout << root->ptrList[ RIGHT ]->content << endl ;
      if ( root->ptrList[ RIGHT ]->mType == LEFT_PAREN )  { // ( a . (
        root->ptrList[ RIGHT ]->content = ".(" ;
        ReadSExp( level + 1, root->ptrList[ RIGHT ] ) ;
      } // if
      else if ( root->ptrList[ RIGHT ]->mType == QUOTE ) {
        // ( a . '
        root->ptrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
        root->ptrList[ RIGHT ]->ptrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
        ReadSExp( level + 1, root->ptrList[ RIGHT ] ) ;
        hasRightParen = false ;
      } // else if

      sc.acceptType[ DOT ] = true ; // it's a trap!
      sc.acceptType[ RIGHT_PAREN ] = true ; // ( ')
    } // if
    else if ( root->ptrList[ RIGHT ]->mType == QUOTE ) {
      root->ptrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ; // ( a .(
      root->ptrList[ RIGHT ]->ptrList[ LEFT ] = new Token( LEFT_PAREN, "(" ) ; // ( a .( (
      thisNode = root->ptrList[ RIGHT ]->ptrList[ LEFT ] ;
      thisNode->ptrList[ LEFT ] = new Token( QUOTE, "quote" ) ;
      thisNode->ptrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      sc.acceptType[ DOT ] = false ; // ( '.
      sc.acceptType[ RIGHT_PAREN ] = false ; // ( ')
      thisNode->ptrList[ RIGHT ]->ptrList[ LEFT ] = ReadToken() ;
      if ( thisNode->ptrList[ RIGHT ]->ptrList[ LEFT ]->mType == LEFT_PAREN
           || thisNode->ptrList[ RIGHT ]->ptrList[ LEFT ]->mType == QUOTE )
        ReadSExp( level + 1, thisNode->ptrList[ RIGHT ]->ptrList[ LEFT ] ) ;
      //else {
      ReadSExp( level + 1, root->ptrList[ RIGHT ] ) ;
      hasRightParen = false ;
      //} // else
      // hasRightParen = false ;
    } // else if
    else if ( root->ptrList[ RIGHT ]->mType != RIGHT_PAREN ) {
      thisNode = root->ptrList[ RIGHT ] ;
      root->ptrList[ RIGHT ] = new Token( LEFT_PAREN, ".(" ) ;
      root->ptrList[ RIGHT ]->ptrList[ LEFT ] = thisNode ;
      if ( thisNode->mType == LEFT_PAREN )
        ReadSExp( level + 1, thisNode ) ;
      ReadSExp( level + 1, root->ptrList[ RIGHT ] ) ;
      // ( a b c d )
      // when d in, d will steal root's ')'
      // so we must told root it's R-Paren had been stolen
      hasRightParen = false ;
    } // else if
  } // if

  // while ( hasRightParen && root->ptrList[ RIGHT ]->mType != RIGHT_PAREN && ReadToken()->mType != RIGHT_PAREN ) ;

  while ( hasRightParen && root->ptrList[ RIGHT ]->mType != RIGHT_PAREN ) {
    Token * dummy = ReadToken() ;
    if ( dummy->mType != RIGHT_PAREN )
      throw OurException( dummy, REQ_RIGHT_P ) ;
    else
      return ;
  } // while

  return ;
} // ReadSExp()

void FixToken( Token * _t ) {
  if (_t->mType == NIL) {
    _t->content = "nil";
  } // if
  else if (_t->mType == QUOTE)
    _t->content = "quote";
  else if (_t->mType == T) {
    _t->content = "#t";
  } // else if
  else if (_t->mType == INT) {
    stringstream ss;
    ss << atoi(_t->content.c_str());
    _t->content = ss.str();
  } // else if
  else if (_t->mType == FLOAT) {
    char floatBuf[32] = { '\0' };
    //The floating number will round off here!!!
    sprintf(floatBuf, "%.03f", atof(_t->content.c_str()));
    _t->content = floatBuf;
  } // else if
  else if ( _t->mType == SYMBOL ) {
    if (_t->content == "nil")
      _t->mType = NIL;
  } // else if
} // FixToken()

void PrintSExp( Token * _tokenList, int level, bool isLeft, stringstream & ss ) {
  if ( _tokenList->mType == LEFT_PAREN && isLeft ) {
    ss << "( " ;
    if ( _tokenList->ptrList[ LEFT ] != NULL ) {
      PrintSExp( _tokenList->ptrList[ LEFT ], level + 1, true, ss ) ;
    } // if
    if ( _tokenList->ptrList[ RIGHT ] != NULL && _tokenList->ptrList[ RIGHT ]->mType != RIGHT_PAREN )
      PrintSExp( _tokenList->ptrList[ RIGHT ], level, false, ss ) ;
    for ( int i = 0 ; i < ( level - 1 ) * 2 ; ++ i )
      ss << ' ' ;
    ss << ')' << endl ;
  } // if
  else {
    if ( _tokenList->content == ".(" ) {
      for ( Token * seek = _tokenList ; seek != NULL && seek->mType != RIGHT_PAREN ; seek = seek->ptrList[ RIGHT ] ) {
        if ( seek->content == ".(" ) {
          if ( seek->ptrList[ LEFT ]->mType == LEFT_PAREN ) {
            for ( int i = 0 ; i < level * 2 ; ++ i )
              ss << ' ' ;
            PrintSExp( seek->ptrList[ LEFT ], level + 1, true, ss ) ;
          } // if
          else {
            for ( int i = 0 ; i < level * 2 ; ++ i )
              ss << ' ' ;
            ss << seek->ptrList[ LEFT ]->content << endl ;
          } // else
        } // if
        else if ( seek->mType != NIL ) {
          for ( int i = 0 ; i < level * 2 ; ++ i )
            ss << ' ' ;
          ss << '.' << endl ;
          for ( int i = 0 ; i < level * 2 ; ++ i )
            ss << ' ' ;
          ss << seek->content << endl ;
        } // else
      } // for
    } // if
    else {
      if ( !isLeft && _tokenList->mType != NIL ) {
        for ( int i = 0 ; i < level * 2 ; ++ i )
          ss << ' ' ;
        ss << '.' << endl ;
        for ( int i = 0 ; i < level * 2 ; ++ i )
          ss << ' ' ;
        ss << _tokenList->content << endl ;
      } // if
      else if ( isLeft )
        ss << _tokenList->content << endl ;
    } // else
  } // else
} // PrintSExp()

void DebugPrint( Token * root ) {
  FixToken( root ) ;
  //cout << root->content << " " << root->mType << endl ;
  if ( root->ptrList[LEFT] != NULL ) {
    //cout << "L " ;
    DebugPrint( root->ptrList[ LEFT ] ) ;
    //cout << "B" << endl ;
  } // if
  if ( root->ptrList[RIGHT] != NULL ) {
    //cout << "R " ;
    DebugPrint( root->ptrList[ RIGHT ] ) ;
    //cout << "B" << endl ;
  } // if
} // DebugPrint

int main() {
  /*Entry point here.*/
  scanf( "%*d%*c" ) ;
  cout << "Welcome to OurScheme!" << endl << endl;
  Token * thisToken = NULL ;
  while (!gProgEnd) {
    cout << "> ";
    try {
      sc.acceptType[ DOT ] = false ;
      sc.acceptType[ RIGHT_PAREN ] = false ;
      thisToken = ReadToken() ;
      stringstream ss ;
      if ( thisToken->mType == LEFT_PAREN || thisToken->mType == QUOTE ) {
        sc.hasOutput = false ;
        ReadSExp( 0, thisToken ) ;
      } // if

      DebugPrint( thisToken ) ;
      PrintSExp( thisToken, 1, true, ss ) ;
      if ( ss.str() == "( exit\n)\n" )
        gProgEnd = true ;
      else
        cout << ss.str() << endl ;
      sc.hasOutput = true ;
      gLine = 1 ;
      gCol = 0 ;
    } catch (OurException e) {
      printf("%s\n\n", e.mErrMsg.c_str());
      sc.hasOutput = false ;
    } // try-catch
  } // while

  printf("Thanks for using OurScheme!");
  return 0;
}

