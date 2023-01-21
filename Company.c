#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "myMacros.h"
#include "Company.h"
#include "Airport.h"
#include "General.h"
#include "fileHelper.h"

static const char* sortOptStr[eNofSortOpt] = {
	"None","Hour", "Date", "Airport takeoff code", "Airport landing code" };


int	initCompanyFromFile(Company* pComp, AirportManager* pManaer, const char* fileName)
{
	L_init(&pComp->flighDateList);
	if (loadCompanyFromFile(pComp, pManaer, fileName))
	{
		initDateList(pComp);
		return 1;
	}
	
	//uncomment to use compressed reading from file.
	/*if (loadCompressedCompanyFromFile(pComp, pManaer, fileName))
	{
		initDateList(pComp);
		return 1;
	}*/

	return 0;
}

void	initCompany(Company* pComp,AirportManager* pManaer)
{
	printf("-----------  Init Airline Company\n");
	L_init(&pComp->flighDateList);
	
	pComp->name = getStrExactName("Enter company name");
	pComp->flightArr = NULL;
	pComp->flightCount = 0;
}

void	initDateList(Company* pComp)
{	
	for (int i = 0; i < pComp->flightCount; i++)
	{
		if(isUniqueDate(pComp,i))
		{
			char* sDate = createDateString(&pComp->flightArr[i]->date);
			L_insert(&(pComp->flighDateList.head), sDate);
		}
	}
}

int		isUniqueDate(const Company* pComp, int index)
{
	Date* pCheck = &pComp->flightArr[index]->date;
	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (i == index)
			continue;
		if (equalDate(&pComp->flightArr[i]->date,pCheck))
			return 0;
	}
	return 1;
}

int		addFlight(Company* pComp, const AirportManager* pManager)
{

	if (pManager->count < 2)
	{
		printf("There are not enoght airport to set a flight\n");
		return 0;
	}
	pComp->flightArr = (Flight**)realloc(pComp->flightArr, (pComp->flightCount + 1) * sizeof(Flight*));
	if (!pComp->flightArr)
		return 0;
	pComp->flightArr[pComp->flightCount] = (Flight*)calloc(1, sizeof(Flight));
	if (!pComp->flightArr[pComp->flightCount])
		return 0;
	initFlight(pComp->flightArr[pComp->flightCount], pManager);
	if (isUniqueDate(pComp, pComp->flightCount))
	{
		char* sDate = createDateString(&pComp->flightArr[pComp->flightCount]->date);
		L_insert(&(pComp->flighDateList.head), sDate);
	}
	pComp->flightCount++;
	return 1;
}

void	printCompany(const Company* pComp,...)
{
#ifndef DETAIL_PRINT
	printf("%s, Number of Flights: %d\n", pComp->name, pComp->flightCount);
#else
	va_list words;
	char* nextWord;
	printf("%s", pComp->name);
	va_start(words, pComp);
	nextWord = va_arg(words, char*);
	while (nextWord != NULL)
	{
		printf("_%s", nextWord);
		nextWord = va_arg(words, char*);	
	} 
	va_end(words);
	printf("\nHas %d flights\n", pComp->flightCount);
	generalArrayFunction((void*)pComp->flightArr, pComp->flightCount, sizeof(Flight**), printFlightV);
	printf("\nFlight Date List:");
	L_print(&pComp->flighDateList, printStr);
#endif // !DETAIL_PRINT
}

void	printFlightsCount(const Company* pComp)
{
	char codeOrigin[CODE_LENGTH + 1];
	char codeDestination[CODE_LENGTH + 1];

	if (pComp->flightCount == 0)
	{
		printf("No flight to search\n");
		return;
	}

	printf("Origin Airport\n");
	getAirportCode(codeOrigin);
	printf("Destination Airport\n");
	getAirportCode(codeDestination);

	int count = countFlightsInRoute(pComp->flightArr, pComp->flightCount, codeOrigin, codeDestination);
	if (count != 0)
		printf("There are %d flights ", count);
	else
		printf("There are No flights ");

	printf("from %s to %s\n", codeOrigin, codeDestination);
}

int		saveCompressedCompanyToFile(const Company* pComp, const char* fileName) {
	FILE* fp;
	int len = strlen(pComp->name),i;
	unsigned char compFlight[10] = { 0,0,0,0,0,0,0,0,0,0 };
	unsigned char* data = (char*)calloc((len + 1 + 2), sizeof(char)); // len +1  bytes for name , 2 bytes for  flights count , sort and airline name length
	if (!data) {
		printf("Error allocation");
		return 0;
	}
	// len*8 is the amount of bytes we want to shift left in bits.
	*data |= pComp->flightCount << ((len*BYTE_SIZE) + 1 + 7);
	*data |= pComp->sortOpt << ((len * BYTE_SIZE) + 1 + 4);
	*data |= len << ((len * BYTE_SIZE) + 1);
	*data |= *pComp->name;

	fp = fopen(fileName, "wb");
	CHECK_NULL_MSG_CLOSE_FILE(fp,fp);

		
	if (!writeStringToFile(&data, fp, "Error to compress company file!"));
		return 0;
		
	for (i = 0; i < pComp->flightCount; i++) {
		if (!writeStringToFile(compressFlight(pComp->flightArr[i]), fp, "Error compress flight!"));
			return 0;
	}
	fclose(fp);
	return 1;
	
}

int		loadCompressedCompanyFromFile(Company* pComp, const AirportManager* pManager, const char* fileName) {
	FILE* fp;
	unsigned char data[2], name[5], source[CODE_LENGTH + 1] = { 0,0,0 }, dest[CODE_LENGTH + 1] = { 0,0,0 };
	source[CODE_LENGTH] = '\0';
	dest[CODE_LENGTH] = '\0';
	int len,i;
	unsigned int val = 0;

	if (fileName == NULL)
		return 0;
	fp = fopen(fileName, "rb");
	MSG_CLOSE_RETURN_0:(fp);
	
	fread(&data, sizeof(char), 2, fp);
	pComp->flightCount = 0;
	pComp->flightCount |= *data >> 9;
	pComp->sortOpt = 0; 
	pComp->sortOpt |= *data << 9;
	pComp->sortOpt >>= 15;
	len = *data << 12;
	len >>= 12;
	fread(&name, sizeof(char), 5, fp);
	strcpy(pComp, &name);

	pComp->flightArr = calloc(pComp->flightCount, sizeof(Flight*));
	if (!pComp->flightArr) {
		printf("Error allocating memory for flights!\n");
		return 0;
	}

	for (i = 0; i < pComp->flightCount; i++)
	{
		fread(&source, sizeof(char), CODE_LENGTH, fp);
		fread(&dest, sizeof(char), CODE_LENGTH, fp);
		if (checkUniqeCode(&source, pManager) == 1 || checkUniqeCode(&dest, pManager) == 1) {
			printf("Error! Code not found\n");
			return 0;
		}
		strcpy(pComp->flightArr[i]->originCode, &source);
		strcpy(pComp->flightArr[i]->destCode, &dest);
		readIntFromFile(&val, fp, "Error reading date of flight\n");
		pComp->flightArr[i]->date.year |= val >> 14;
		pComp->flightArr[i]->date.month |= val << 18;
		pComp->flightArr[i]->date.month >>= 28;
		pComp->flightArr[i]->date.day |= val << 22;
		pComp->flightArr[i]->date.day >>= 27;
		pComp->flightArr[i]->hour |= val << 27;
		pComp->flightArr[i]->hour >>= 27;

		val = 0;
		*dest = 0x0;
		*source = 0x0;

	}
	
	fclose(fp);
	return 1;
}

int		saveCompanyToFile(const Company* pComp, const char* fileName)
{
	FILE* fp;
	fp = fopen(fileName, "wb");
	MSG_CLOSE_RETURN_0:(fp);
	
	if (!writeStringToFile(pComp->name, fp, "Error write comapny name\n"))
		return 0;

	if (!writeIntToFile(pComp->flightCount, fp, "Error write flight count\n"))
		return 0;

	if (!writeIntToFile((int)pComp->sortOpt, fp, "Error write sort option\n"))
		return 0;

	for (int i = 0; i < pComp->flightCount; i++)
	{
		if (!saveFlightToFile(pComp->flightArr[i], fp))
			return 0;
	}

	fclose(fp);
	return 1;
}

int		loadCompanyFromFile(Company* pComp, const AirportManager* pManager, const char* fileName)
{
	FILE* fp;
	if (fileName == NULL)
		return 0;
	fp = fopen(fileName, "rb");
	MSG_CLOSE_RETURN_0:(fp);

	pComp->flightArr = NULL;


	pComp->name = readStringFromFile(fp, "Error reading company name\n");
	if (!pComp->name)
		return 0;

	if (!readIntFromFile(&pComp->flightCount, fp, "Error reading flight count name\n"))
		return 0;

	int opt;
	if (!readIntFromFile(&opt, fp,"Error reading sort option\n"))
		return 0;

	pComp->sortOpt = (eSortOption)opt;

	if (pComp->flightCount > 0)
	{
		pComp->flightArr = (Flight**)malloc(pComp->flightCount * sizeof(Flight*));
		CHECK_NULL_MSG_CLOSE_FILE(fp, pComp->flightArr);
	}
	else
		pComp->flightArr = NULL;

	for (int i = 0; i < pComp->flightCount; i++)
	{
		pComp->flightArr[i] = (Flight*)calloc(1, sizeof(Flight));
		CHECK_NULL_MSG_CLOSE_FILE(fp, pComp->flightArr[i]);
		
		if (!loadFlightFromFile(pComp->flightArr[i], pManager, fp))
			return 0;
	}

	fclose(fp);
	return 1;
}

void	sortFlight(Company* pComp)
{
	pComp->sortOpt = showSortMenu();
	int(*compare)(const void* air1, const void* air2) = NULL;

	switch (pComp->sortOpt)
	{
	case eHour:
		compare = compareByHour;
		break;
	case eDate:
		compare = compareByDate;
		break;
	case eSorceCode:
		compare = compareByCodeOrig;
		break;
	case eDestCode:
		compare = compareByCodeDest;
		break;
	
	}

	if (compare != NULL)
		qsort(pComp->flightArr, pComp->flightCount, sizeof(Flight*), compare);

}

void	findFlight(const Company* pComp)
{
	int(*compare)(const void* air1, const void* air2) = NULL;
	Flight f = { 0 };
	Flight* pFlight = &f;


	switch (pComp->sortOpt)
	{
	case eHour:
		f.hour = getFlightHour();
		compare = compareByHour;
		break;
	case eDate:
		getchar();
		getCorrectDate(&f.date);
		compare = compareByDate;
		break;
	case eSorceCode:
		getchar();
		getAirportCode(f.originCode);
		compare = compareByCodeOrig;
		break;
	case eDestCode:
		getchar();
		getAirportCode(f.destCode);
		compare = compareByCodeDest;
		break;
	}

	if (compare != NULL)
	{
		Flight** pF = bsearch(&pFlight, pComp->flightArr, pComp->flightCount, sizeof(Flight*), compare);
		if (pF == NULL)
			printf("Flight was not found\n");
		else {
			printf("Flight found, ");
			printFlight(*pF);
		}
	}
	else {
		printf("The search cannot be performed, array not sorted\n");
	}

}

eSortOption showSortMenu()
{
	int opt;
	printf("Base on what field do you want to sort?\n");
	do {
		for (int i = 1; i < eNofSortOpt; i++)
			printf("Enter %d for %s\n", i, sortOptStr[i]);
		scanf("%d", &opt);
	} while (opt < 0 || opt >eNofSortOpt);

	return (eSortOption)opt;
}

void	freeCompany(Company* pComp)
{
	generalArrayFunction((void*)pComp->flightArr, pComp->flightCount, sizeof(Flight**), freeFlight);
	free(pComp->flightArr);
	free(pComp->name);
	L_free(&pComp->flighDateList, freePtr);
}
