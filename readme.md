﻿// ===========================================================================

How to write OurScheme (Latest modification : 02/06, 2017)

// ===========================================================================

Main program for Project 1

  Print 'Welcome to OurScheme!'

  repeat
  
    Print '> '

    ReadSExp(exp);
    
    if no error
      then PrintSExp(exp);
    else 
      PrintErrorMessage() ;
      
    until (OR (user entered '(exit)')
              (END-OF-FILE encountered)
          )

    Print 'Thanks for using OurScheme!' or EOF error message

Main program for the remaining projects

  Print 'Welcome to OurScheme!'
  
  repeat
  
    Print : '> '
    
    ReadSExp( s_exp ); 
    
    if no error
      then result <- EvalSExp( s_exp );
           if error
             PrintErrorMessage();
           else
             PrintSExp( result ) ;
    else PrintErrorMessage() ;
    
  until user has just entered LEFT_PAREN "exit" RIGHT_PAREN
         or
         EOF encountered
  
  Print 'Thanks for using OurScheme!' or EOF error message

�@�B Read in an S-expression

First, try to read in an S-expression.

terminal :

  LEFT-PAREN  // '('
  RIGHT-PAREN // ')'
  INT         // e.g., '123', '+123', '-123'
  STRING     // "This is an example of a string." 
              // (strings do not extend across lines)
              // OurScheme��string��C/Java��printf()��escape�������A���u����'\n', '\"', '\t'
              // �P'\n' �F �p�G'\'�r�����᪺�r�����O'n', '"', 't', ��'\'�A��(�Ĥ@��)'\'�r���N�L
              // �S��N�q(�ӥu�O�@�Ӵ��q�r��)�C
              // (�ҡG "There is an ENTER HERE>>\nSee?!", "Use '\"' to start and close a string."
              //       "OurScheme allows the use of '\\n', '\\t' and  '\\"' in a string."
              //       "Please enter YES\NO below this line >\n" 
              //       "You need to handle this \\"        "You also need to handle this\"" )
  DOT         // '.'
  FLOAT       // '123.567', '123.', '.567', '+123.4', '-.123'
  NIL         // 'nil' or '#f', but not 'NIL' nor 'nIL'
  T           // 't' or '#t', but not 'T' nor '#T'
  QUOTE       // '
  SYMBOL      // a consecutive sequence of printable characters 
              // that are not numbers,
              // and do not contain '(', ')', 
              // single-quote, double-quote and white-spaces ;
              // Symbols are case-sensitive 
              // (i.e., uppercase and lowercase are different);

Note :

  With the exception of strings, token are separated by the following "separators" :
    (a) one or more white-spaces
    (b) '('                             (note : '(' is a token by itself)
    (c) ')'                             (note : ')' is a token by itself)
    (d) the single-quote character (')  (note : this is a token by itself)
    (e) the double-quote character (")  (note : this starts a STRING)
    (f) semi-colon (;)                     (note : this is the start of a line comment)

Examples :

  '3.25' is a FLOAT.
  '3.25a' is a SYMBOL.
  'a.b' is a SYMBOL.
  '#f' is NIL
  '#fa' (alternatively, 'a#f') is a SYMBOL.

Note :

  '.' can mean several things : 
  it is either part of a FLOAT or part of a SYMBOL or a DOT.
    
  It means a DOT only when it "stands alone".
  
  '#' can also mean two things :
    it is either part of NIL (or T) or part of a SYMBOL.
  
  It is part of NIL (or T) only when it is '#t' or '#f' that 
  "stand alone".
  
<S-exp> ::= <ATOM> 
            | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] 
              RIGHT-PAREN
            | QUOTE <S-exp>
            
<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
            | NIL | T | LEFT-PAREN RIGHT-PAREN

Once the attempt to read in an S-expression fails, the line containing the error-char is ignored.  Start to read in an S-expression from the next input line.

> (t . nil . (1 2 3))
ERROR (unexpected character) : line 1 column 10 character '.'

> (12 (    . 3))
ERROR (unexpected character) : line 1 column 11 character ' '

> ())
nil

> ERROR (unexpected character) : line 1 column 1 character ')'

> (1 2 3) )
( 1
  2
  3
)

> ERROR (unexpected character) : line 1 column 2 character ')'

>'(1 2 3) )
( quote
  ( 1
    2
    3
  )
)

> ERROR (unexpected character) : line 1 column 2 character ')'

�G�B Always check the syntax of the user��s input; Must make sure that it is an 
     S-expression before evaluating it.

   User input �i��|�����T��syntax error������message(���d��)�p�U�G
      ERROR (unexpected character) : line 1 column 2 character ')'
      ERROR (unexpected character) : line 3 column 27 LINE-ENTER encountered
      ERROR : END-OF-FILE encountered when there should be more input

�T�B The part of eval() concerning error messages : // Note : once an error occurs,
                                                              //    the call to eval() is over
if what is being evaluated is an atom but not a symbol

  return that atom
  
else if what is being evaluated is a symbol 

  check whether it is bound to an S-expression or an internal function

  if unbound
    ERROR (unbound symbol) : abc
  else 
    return that S-expression or internal function (i.e., its binding)

else // what is being evaluated is (...) ; we call it the main S-expression below
      // this (...) cannot be nil (nil is an atom)
  if (...) is not a (pure) list
    ERROR (non-list) : (...)  // (...)�npretty print

  else if first argument of (...) is an atom ��, which is not a symbol
    ERROR (attempt to apply non-function) : ��

  else if first argument of (...) is a symbol SYM

    check whether SYM is the name of a function (i.e., check whether �uSYM has a
                                      binding, and that binding is an internal function�v)

    if SYM is the name of a known function

      if the current level is not the top level, and SYM is 'clean-environment' or    
          or 'define' or�@'exit'

        ERROR (clean-environment format) / ERROR (define format) / ERROR (level of exit)
        // Project 2 ��test data�W�w�n ERROR (clean-environment/define format)�A�Ȥ��復�C

      if SYM is 'define' or 'set!' or 'let' or 'cond' or 'lambda'

        check the format of this expression // �`�N�G���ɩ|��check num-of-arg
        // (define symbol    // �`�N�G�u��ŧi�γ]�w �Dprimitive��symbol (�o�Ofinal decision!)
        //         S-expression
        // )
        // (define ( one-or-more-symbols )
        //           one-or-more-S-expressions
        // )
        // (set! symbol
        //       S-expression
        // )
        // (lambda (zero-or-more-symbols)
        //           one-or-more-S-expressions
        // )
        // (let (zero-or-more-PAIRs)
        //        one-or-more-S-expressions
        // )
        // (cond one-or-more-AT-LEAST-DOUBLETONs
        // )
        // where PAIR df= ( symbol S-expression )
        //        AT-LEAST-DOUBLETON df= a list of two or more S-expressions

        if format error (�]�Aattempting to redefine system primitive) 
          ERROR (COND format) : <the main S-exp> 
          or
          ERROR (DEFINE format) : <the main S-exp> // ���i��O�]��redefining primitive���G
          or
          ERROR (SET! format) : <the main S-exp>    // ���i��O�]��redefining primitive���G
          or
          ERROR (LET format) : <the main S-exp>     // ���i��O�]��redefining primitive���G
          or
          ERROR (LAMBDA format) : <the main S-exp>  // ���i��O�]��redefining primitive���G

        evaluate ( ... ) 
        // for 'cond', there may be ERROR (COND did not return value) : <the main S-exp>

        return the evaluated result (and exit this call to eval())

      else if SYM is 'if' or 'and' or 'or'

        check whether the number of arguments is correct

        if number of arguments is NOT correct
          ERROR (incorrect number of arguments) : if

        evaluate ( ... ) 

        return the evaluated result (and exit this call to eval())

      else // SYM is a known function name 'abc', which is neither 
            // 'define' nor 'let' nor 'cond' nor 'lambda'

        check whether the number of arguments is correct

        if number of arguments is NOT correct
          ERROR (incorrect number of arguments) : abc

    else // SYM is 'abc', which is not the name of a known function

      ERROR (unbound symbol) : abc
      or
      ERROR (attempt to apply non-function) : �� // �� is the binding of abc

  else // the first argument of ( ... ) is ( �C�C�C ), i.e., it is ( ( �C�C�C ) ...... )

    evaluate ( �C�C�C )

    // if any error occurs during the evaluation of ( �C�C�C ), we just output an
    // an appropriate error message, and we will not proceed any further

    if no error occurs during the evaluation of ( �C�C�C ) 

      check whether the evaluated result (of ( �C�C�C )) is an internal function

      if the evaluated result (of ( �C�C�C )) is an internal function

        check whether the number of arguments is correct

        if num-of-arguments is NOT correct
          ERROR (incorrect number of arguments) : name-of-the-function
          or
          ERROR (incorrect number of arguments) : lambda expression 
                                                        // in the case of nameless functions

      else // the evaluated result (of ( �C�C�C )) is not an internal function
        ERROR (attempt to apply non-function) : �� //  �� is the evaluated result
    
  eval the second argument S2 of (the main S-expression) ( ... )

  if the type of the evaluated result is not correct 
    ERROR (xxx with incorrect argument type) : the-evaluated-result
    // xxx must be the name of some primitive function!

  if no error
    eval the third argument S3 of (the main S-expression) ( ... )

  if the type of the evaluated result is not correct 
    ERROR (xxx with incorrect argument type) : the-evaluated-result

  ...

  if no error

    apply the binding of the first argument (an internal function) to S2-eval-result, 
    S3-eval-result, ... 

    if no error
      if there is an evaluated result to be returned
        return the evaluated result
      else
        ERROR (no return result) : name-of-this-function
        or
        ERROR (no return result) : lambda expression // if there is such a case ...

end // else what is being evaluated is (...) ; we call it the main S-expression

Note : 

1. error message���u��L�v

�p�G�A���t�θI��@��error�B�ӥH�Weval��algorithm���惡error�u�Ӧ���error message�v�èS���W�d(�o���I���Oif-then-else-if-then-...-else-if-then-else�����̫ᨺ�ӡuelse�v)�A�A�Noutput

                ERROR : aaa

�䤤aaa�Ouser input���u�X���D�����ӡu�Qevaluate��function�v��first argument�v�C

��A�̷ӥH�Weval��algorithm��evaluate an S-expression�ɡA�A�|���_���nevaluate a function�A�@���o�ءuproject�å��W�d��error�v�o�͡A�u��ɡv���ӳQevaluate��function��first argument�N�O�o�̩ҿת�aaa�C

// �uproject���W�d�v df= project��(�Ptest data��)������o��error�A�������N�O��error
//                          |                           // i.e., OR
//                          project��������o��error�A���S��error message���ӬOԣ

  e.g.,

  > (/ 3 0)
  ERROR : /

2. Some examples of error messages

> (car nil)
ERROR (car with incorrect argument type) : nil

> (define (f a) (cons a a))
f defined

> (f 5)
( 5
  .
  5
)

> (f 5 a)
ERROR (incorrect number of arguments) : f

> (define (ff a) (g a a))
ff defined

> (define (g a) (cons a a))
g defined

> (ff 5)
ERROR (incorrect number of arguments) : g

> (define (f a) (cons a a a))
f defined

> (f 5)
ERROR (incorrect number of arguments) : cons

> (CONS 3 4)      
ERROR (unbound symbol) : CONS

> (cons hello 4)
ERROR (unbound symbol) : hello

> hello
ERROR (unbound symbol) : hello

> (CONS hello there)
ERROR (unbound symbol) : CONS

> (cons 1 2 3)
ERROR (incorrect number of arguments) : cons

> (3 4 5)
ERROR (attempt to apply non-function) : 3

> (cons 3 
        (4321 5))
ERROR (attempt to apply non-function) : 4321

> (define a 5)
5

> (a 3 a)
ERROR (attempt to apply non-function) : 5

> (* 3 "Hi")
ERROR (* with incorrect argument type) : "Hi"

> (string>? 15 "hi")
ERROR (string>? with incorrect argument type) : 15

> (+ 15 "hi")
ERROR (+ with incorrect argument type) : "hi"

> (string>? "hi" "there" a)
ERROR (string>? with incorrect argument type) : 5

> (string>? "hi" "there" about)
ERROR (unbound symbol) : about

> (cond ((> 3 4) 'bad)
        ((> 4 5) 'bad)
  )
ERROR (return value undefined) : cond

> (cond ((> y 4) 'bad)
        ((> 4 3) 'good)
  )
ERROR (unbound symbol) : y

3.value and binding

Lisp and Scheme ����@�ӷ����G

                    �S���uvalue�v�I �u���ubinding�v�I

�]�N�O���G
           �S���usymbol��value�v�o�^�ơI �u���usymbol��binding�v�I

  �� Symbol��binding�i��O�@��S-expression (which is basically a structure
     of symbols)�A�]�i��O�@��(�ҿת�)internal function�C

  �� Internal functions���ƥ�system define�n���A�]��user define���C

  �� evaluate �@�ӡu�Dsymbol��atom�v  �����G  �O  ����atom

  �� evaluate �@��symbol  �����G  �O  ����symbol��binding

  �� evaluate �@��list  �����G  �O  apply �uevaluate��list��first argument
     �ұo�����G�v(which is supposedly an internal function)  ��
     �uevaluate��list����L argument �ұo�����G�v

�g�ѨϥάY��system defined ��"�F�F" (�p'define')�A�ڭ̥i�H����symbol��binding�C
�� �ڭ̯���ܡu���system�wdefine�n��symbol�v��binding�ܡH

�ҡG how about these�H

  > (define define 3)
  ???

  > (define exit 3)
  ???

  > (let ((cons car)) (cons '(1 2)))
  ???

Petite Scheme ���\�p���I  OurScheme�n���n�H

���סG �ڭ� �����\ ����"primitive symbol"��binding�I

// ���H���ݳo�椣��G (define 3 4)
// ���שT�M�O����A����]�O�G 'define'�u����ܡusymbol�v��binding

// �]���H���ݳo�椣��G (define nil 4)
// ���שT�M�O����A����]�O�G 'nil'���O�usymbol�v

But note that below is OK.

> (define myCons cons)
myCons defined

> myCons
#<internal function>

> (myCons 3 5)
( 3
  .
  5
)

> (define a (myCons car cdr))
a defined

> a
( #<internal function>
  .
  #<internal function>
)

> ((car a) '(1 2 3))
1

> ((cdr a) '(1 2 3))
( 2
  3
)

Explanation :

  #<internal function> is no different from "others" such as 3, 5,

  "Hi", (1 2 3) and (1 (2 3) "Hi), and should be treated in the same way.

  (But of course, #<internal function> is capable of doing something

   these "others" cannot do. But that is a different story.)

4. Expected argument type

Below, the word 'symbol' should be taken to mean : a symbol that
is not a primitive symbol (i.e., it is not a pre-defined symbol)

�� 'car' expects its argument to be a cons-cell.

�� 'cdr' expects its argument to be a cons-cell.

�� 'quote' expects its argument to be an S-expression.

�� 'define' expects that either its first argument is a symbol or its first argument 
   is a list of one or more symbols.

�� 'lambda' expects that its first argument is a list of zero or more symbols.

�� 'let' expects its first argument to be a list of one or more pairs, with the first 
   element of each pair being a symbol.

�� '+', '-','*','/' expect their arguments to be numbers.

�� '>', '>=','<','<=','=' expect their arguments to be numbers.

�� 'string-append' and 'string>?' expect their arguments to be strings.

�� 'set!', 'set-car!' and 'set-cdr!' expect their first argument to be a symbol.

�� 'display-string' expects its argument to be a string.

�� 'load' and 'make-directory' expect their arguments to be strings.

�� In all other cases, S-expressions or internal functions are expected as arguments.
