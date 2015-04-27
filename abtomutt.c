/* abtomutt.c
 *
 * Copyright 2008, Stephen Fisher
 *
 * This file is part of Address Book to Mutt.
 * 
 * Address Book to Mutt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Address Book to Mutt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with abtomutt.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Modified March 2013, Brad McDuffie
 * I added functions and cleaned up some errors.
 */

#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>
#include <AddressBook/ABAddressBookC.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

char *get_result_from_cfstring(CFStringRef name);
void print_results(CFStringRef firstName, CFStringRef lastName, CFStringRef emailAddr, CFStringRef companyName);

int
main(int argc, char *argv[])
{
	CFStringRef searchTerm, firstName, lastName, emailAddr, companyName;
	ABMultiValueRef emailAddrMulti;
	CFArrayRef nameArray, personFound, record;
	CFIndex idx, firstNameLen, lastNameLen, emailAddrLen, companyNameLen;
    /* Ok, this caused a warning in the build, so I fixed it
    CFIndex numFound;
    * abtomutt.c:98: warning: format ‘%d’ expects type ‘int’, but argument 2 has type ‘CFIndex’
    * -Brad
    */
    int numFound;
	Boolean ret;
	ABAddressBookRef AB;
	ABSearchElementRef names[2], element;
	char *firstNameResult, *lastNameResult, *emailAddrResult;
	char *companyNameResult;

	AB = ABGetSharedAddressBook();

	if(!argv[1]) {
#ifdef HAVE_CONFIG_H
		printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
#endif
		printf("usage: abtomutt <search string>\n");
		return -1;
	}

	searchTerm = CFStringCreateWithCString(kCFAllocatorDefault,
					       argv[1],
					       kCFStringEncodingASCII);
	if(!searchTerm) {
		fprintf(stderr, "Unable to copy command line parameter.\n");
		return -1;
	}

	names[0] =
		ABPersonCreateSearchElement(kABFirstNameProperty, NULL,
					    NULL, searchTerm,
					    kABContainsSubStringCaseInsensitive);

	names[1] =
		ABPersonCreateSearchElement(kABLastNameProperty, NULL, NULL,
					    searchTerm,
					    kABContainsSubStringCaseInsensitive);
	
	CFRelease(searchTerm);

	nameArray = CFArrayCreate(kCFAllocatorDefault, (const void**)names, 2,
				  NULL);

	element = ABSearchElementCreateWithConjunction(kABSearchOr, nameArray);

	CFRelease(nameArray);

	personFound = ABCopyArrayOfMatchingRecords(AB, element);

	CFRelease(element);

	numFound = CFArrayGetCount(personFound);

	if(numFound == 0) {
		printf("No matches found.\n");
		return -1;
	} 
    /* I removed the Found X matches because it was broken here
     * -Brad
     */

	for(idx = 0; idx < numFound; idx++) {
		record = CFArrayGetValueAtIndex(personFound, idx);
		
		firstName =
			ABRecordCopyValue((ABRecordRef)record,
					  kABFirstNameProperty);

		lastName =
			ABRecordCopyValue((ABRecordRef)record,
					  kABLastNameProperty);

		companyName =
			ABRecordCopyValue((ABRecordRef)record,
					  kABOrganizationProperty);

		emailAddrMulti =
			ABRecordCopyValue((ABRecordRef)record,
					  kABEmailProperty);

        /* This is the section that finds the email addresses and
         * prints them out.  The original just returned one value,
         * but I needed it to do multiple addresses for each person.
         * So, I moved the print code into a function, setup a 
         * count variable (emailCount) and looped over the values.
         * --Brad
         */
        int emailCount;
        emailCount = ABMultiValueCount(emailAddrMulti);
        if (emailCount > 0) {
            int c;
            for(c = 0; c< emailCount; c++) {
                emailAddr = ABMultiValueCopyValueAtIndex(emailAddrMulti, c);
                print_results(firstName, lastName, emailAddr, companyName);
            }
        }


		CFRelease(record);

		if(!emailAddr)
			/* Restart for() loop to check for other entries that
			 * do contain an e-mail address. */
			continue;
	}

	CFRelease(personFound);

	return 0;
}

/* Return value must be free()'d! */
char *
get_result_from_cfstring(CFStringRef name)
{
	CFIndex nameLen;
	char *nameResult;
	Boolean ret;

	if(!name) {
		nameResult = NULL;
		return nameResult;
	}

	nameLen = CFStringGetLength(name) + 1;
	nameResult = malloc(nameLen);
	if(!nameResult) {
		fprintf(stderr, "Unable to allocate memory in file %s at line"
			" %d.\n", __FILE__, __LINE__);
		exit(-1);
	}
	
	ret = CFStringGetCString(name, nameResult, nameLen,
				 kCFStringEncodingASCII);
	if(!ret) {
		fprintf(stderr, "Unable to create CString from search results"
			" in file %s at line %d.\n", __FILE__, __LINE__);
		exit(-1);
	}

	return nameResult;
}

/* I moved Steve's code for printing the results into a function
 * so that it would be easier to loop over all of the addresses
 * for each person found. -Brad 
 */
void print_results(CFStringRef firstName, CFStringRef lastName, CFStringRef emailAddr, CFStringRef companyName)
{
	char *firstNameResult, *lastNameResult, *emailAddrResult;
	char *companyNameResult;
	firstNameResult = get_result_from_cfstring(firstName);
	lastNameResult = get_result_from_cfstring(lastName);
	emailAddrResult = get_result_from_cfstring(emailAddr);
	companyNameResult = get_result_from_cfstring(companyName);

	printf("%s\t", emailAddrResult);

	if(firstNameResult)
		printf("%s", firstNameResult);

	if(lastNameResult)
		printf(" %s", lastNameResult);

	if(companyNameResult) {
		printf("\t%s", companyNameResult);
		CFRelease(companyName);
		free(companyNameResult);
	}

	printf("\n");

	if(firstName)
		CFRelease(firstName);
	if(lastName)
		CFRelease(lastName);
	if(emailAddr)
		CFRelease(emailAddr);
	
	if(firstNameResult)
		free(firstNameResult);
	if(lastNameResult)
		free(lastNameResult);
	if(emailAddrResult)
		free(emailAddrResult);
}
