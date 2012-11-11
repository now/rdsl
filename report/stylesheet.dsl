<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
    <!ENTITY html-ss
      PUBLIC "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN" CDATA dsssl>
    <!ENTITY print-ss
      PUBLIC "-//Norman Walsh//DOCUMENT DocBook Print Stylesheet//EN" CDATA dsssl>
    ]>
    
    <style-sheet>
    <style-specification id="print" use="print-stylesheet">
    <style-specification-body> 
    
	;; customize the print stylesheet
    	;; paper size
	(define %paper-type% "A4")
	;; how to justify paragraphs
	(define %default-quadding% 'justify)
	;; put footnotes at the bottom of each page
	;; sadly doesn't work with jadetex it seems (only tex?)
	(define bop-footnotes #t)
	;; First line start-indent for paragraphs (other than the first)
	(define %para-indent% 1pi)
	;; Distance between paragraphs
	(define %para-sep% 0pt)
	;; Extra start-indent for block-elements
	(define %block-start-indent% 2pi)
	;; Distance between block-elements
	(define %block-sep% 8pt)
    
    </style-specification-body>
    </style-specification>
    <style-specification id="html" use="html-stylesheet">
    <style-specification-body> 
    
    ;; customize the html stylesheet
    
    </style-specification-body>
    </style-specification>
    <external-specification id="print-stylesheet" document="print-ss">
    <external-specification id="html-stylesheet" document="html-ss">
</style-sheet>
