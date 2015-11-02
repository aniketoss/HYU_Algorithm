#!/bin/bash
#	Test expr statment

a=`expr 4 "*" 5` # `expr 4 * 5` : syntax error , 
		 # using escaepe(\), or double-qoutation

echo "4 * 5 = $a"
