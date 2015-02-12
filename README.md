abtomutt
========
abtomutt is software written by Steven Fisher (http://sourceforge.net/projects/abtomutt/) to look up email addresses
for contacts in the Apple Address book (now Contacts).  I've merely just taken his abtomutt.c file and made it print 
out all emails for each matched contact rather than the original, which only printed the first email address found.

I tried to contact Steven Fisher, but his email address that I found in the source code doesn't seem to be working
any more, so I decided to just throw this up on my github account.

Compile like:

clang -o abtomutt -framework CoreFoundation -framework AddressBook abtomutt.c

