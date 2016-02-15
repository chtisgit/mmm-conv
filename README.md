# mmm-conv
Converts driving license test questions from a proprietary binary format to javascript

MMM Software produces a program that allows users to practice
driving license test questions (these are specific to Austria).

These questions are encoded in a binary format, of which I could decode the most
important bits.

mmm-conv converts the files "Themen.mmm" and "Fr_Deu.mmm" to a "questions.js" file, 
which defines the variables questions and topics (currently only the former).
These variables contain arrays of the questions (with their answers) and the
topics, respectively.

Together with index.html (which could be extended further), this could form a
web app, which is no longer dependent on the proprietary CD/format.
