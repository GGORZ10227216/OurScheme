// ===========================================================================

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

一、 Read in an S-expression

First, try to read in an S-expression.

terminal :

    LEFT-PAREN  // '('
    RIGHT-PAREN // ')'
    INT         // e.g., '123', '+123', '-123'
    STRING     // "This is an example of a string." 
               // (strings do not extend across lines)
               // OurScheme的string有C/Java的printf()的escape的概念，但只限於'\n', '\"', '\t'
               // 與'\n' ； 如果'\'字元之後的字元不是'n', '"', 't', 或'\'，此(第一個)'\'字元就無
               // 特殊意義(而只是一個普通字元)。
               // (例： "There is an ENTER HERE>>\nSee?!", "Use '\"' to start and close a string."
               //       "OurScheme allows the use of '\\n', '\\t' and  '\\"' in a string."
               //       "Please enter YES\NO below this line >\n" 
               //       "You need to handle this \\"        "You also need to handle this\"" )
    DOT        // '.'
    FLOAT      // '123.567', '123.', '.567', '+123.4', '-.123'
    NIL        // 'nil' or '#f', but not 'NIL' nor 'nIL'
    T          // 't' or '#t', but not 'T' nor '#T'
    QUOTE      // '
    SYMBOL     // a consecutive sequence of printable characters 
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

二、 Always check the syntax of the user’s input; Must make sure that it is an 
     S-expression before evaluating it.

   User input 可能會有的三種syntax error的相關message(的範例)如下：
      ERROR (unexpected character) : line 1 column 2 character ')'
      ERROR (unexpected character) : line 3 column 27 LINE-ENTER encountered
      ERROR : END-OF-FILE encountered when there should be more input

三、 The part of eval() concerning error messages : // Note : once an error occurs,
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
    ERROR (non-list) : (...)  // (...)要pretty print

  else if first argument of (...) is an atom ☆, which is not a symbol
    ERROR (attempt to apply non-function) : ☆

  else if first argument of (...) is a symbol SYM

    check whether SYM is the name of a function (i.e., check whether 「SYM has a
                                      binding, and that binding is an internal function」)

    if SYM is the name of a known function

      if the current level is not the top level, and SYM is 'clean-environment' or    
          or 'define' or　'exit'

        ERROR (clean-environment format) / ERROR (define format) / ERROR (level of exit)
        // Project 2 的test data規定要 ERROR (clean-environment/define format)，暫不改它。

      if SYM is 'define' or 'set!' or 'let' or 'cond' or 'lambda'

        check the format of this expression // 注意：此時尚未check num-of-arg
        // (define symbol    // 注意：只能宣告或設定 非primitive的symbol (這是final decision!)
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

        if format error (包括attempting to redefine system primitive) 
          ERROR (COND format) : <the main S-exp> 
          or
          ERROR (DEFINE format) : <the main S-exp> // 有可能是因為redefining primitive之故
          or
          ERROR (SET! format) : <the main S-exp>    // 有可能是因為redefining primitive之故
          or
          ERROR (LET format) : <the main S-exp>     // 有可能是因為redefining primitive之故
          or
          ERROR (LAMBDA format) : <the main S-exp>  // 有可能是因為redefining primitive之故

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
      ERROR (attempt to apply non-function) : ☆ // ☆ is the binding of abc

  else // the first argument of ( ... ) is ( 。。。 ), i.e., it is ( ( 。。。 ) ...... )

    evaluate ( 。。。 )

    // if any error occurs during the evaluation of ( 。。。 ), we just output an
    // an appropriate error message, and we will not proceed any further

    if no error occurs during the evaluation of ( 。。。 ) 

      check whether the evaluated result (of ( 。。。 )) is an internal function

      if the evaluated result (of ( 。。。 )) is an internal function

        check whether the number of arguments is correct

        if num-of-arguments is NOT correct
          ERROR (incorrect number of arguments) : name-of-the-function
          or
          ERROR (incorrect number of arguments) : lambda expression 
                                                        // in the case of nameless functions

      else // the evaluated result (of ( 。。。 )) is not an internal function
        ERROR (attempt to apply non-function) : ☆ //  ☆ is the evaluated result
    
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

1. error message之「其他」

如果你的系統碰到一個error、而以上eval的algorithm中對此error「該有何error message」並沒有規範(這有點像是if-then-else-if-then-...-else-if-then-else中的最後那個「else」)，你就output

                ERROR : aaa

其中aaa是user input中「出問題的那個「被evaluate的function」的first argument」。

當你依照以上eval的algorithm來evaluate an S-expression時，你會不斷的要evaluate a function，一旦這種「project並未規範的error」發生，「當時」那個被evaluate的function的first argument就是這裡所謂的aaa。

// 「project未規範」 df= project中(與test data中)未提到這種error，但明明就是個error
//                          |                           // i.e., OR
//                          project中有提到這種error，但沒說error message應該是啥

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

Lisp and Scheme 堅持一個概念：

                    沒有「value」！ 只有「binding」！

也就是說：
           沒有「symbol的value」這回事！ 只有「symbol的binding」！

  ＊ Symbol的binding可能是一個S-expression (which is basically a structure
     of symbols)，也可能是一個(所謂的)internal function。

  ＊ Internal functions有事先system define好的，也有user define的。

  ＊ evaluate 一個「非symbol的atom」  的結果  是  那個atom

  ＊ evaluate 一個symbol  的結果  是  那個symbol的binding

  ＊ evaluate 一個list  的結果  是  apply 「evaluate此list的first argument
     所得的結果」(which is supposedly an internal function)  於
     「evaluate此list的其他 argument 所得的結果」

經由使用某些system defined 的"東東" (如'define')，我們可以改變symbol的binding。
但 我們能改變「原先system已define好的symbol」的binding嗎？

例： how about these？

  > (define define 3)
  ???

  > (define exit 3)
  ???

  > (let ((cons car)) (cons '(1 2)))
  ???

Petite Scheme 允許如此！  OurScheme要不要？

答案： 我們 不允許 改變"primitive symbol"的binding！

// 有人曾問這行不行： (define 3 4)
// 答案固然是不行，但原因是： 'define'只能改變「symbol」的binding

// 也有人曾問這行不行： (define nil 4)
// 答案固然是不行，但原因是： 'nil'不是「symbol」

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

＊ 'car' expects its argument to be a cons-cell.

＊ 'cdr' expects its argument to be a cons-cell.

＊ 'quote' expects its argument to be an S-expression.

＊ 'define' expects that either its first argument is a symbol or its first argument 
   is a list of one or more symbols.

＊ 'lambda' expects that its first argument is a list of zero or more symbols.

＊ 'let' expects its first argument to be a list of one or more pairs, with the first 
   element of each pair being a symbol.

＊ '+', '-','*','/' expect their arguments to be numbers.

＊ '>', '>=','<','<=','=' expect their arguments to be numbers.

＊ 'string-append' and 'string>?' expect their arguments to be strings.

＊ 'set!', 'set-car!' and 'set-cdr!' expect their first argument to be a symbol.

＊ 'display-string' expects its argument to be a string.

＊ 'load' and 'make-directory' expect their arguments to be strings.

＊ In all other cases, S-expressions or internal functions are expected as arguments.
