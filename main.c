#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myMacros.h"
#include "Company.h"
#include "AirportManager.h"
#include "General.h"
#include "main.h"

const char* str[eNofOptions] = { "Add Flight", "Add Airport",
"PrintCompany", "Print all Airports",
	"Print flights between origin-destination",
"Sort Flights", "Search Flight" };


void main(int argc, char* argv[])
{
	AirportManager	manager;
	Company			company;
	char* fileName = "";
	char* binaryFileName = "";

	if(argc != 3)
		initManagerAndCompany(&manager,NULL, &company,NULL);
	else
	{
		fileName = strdup (argv[1]);
		binaryFileName = strdup(argv[2]);
		initManagerAndCompany(&manager, fileName, &company, binaryFileName);
	}

	int option;
	int stop = 0;
	
	do
	{
		option = menu();
		switch (option)
		{
		case eAddFlight:
			if (!addFlight(&company, &manager))
				printf("Error adding flight\n");
			break;


		case eAddAirport:
			if (!addAirport(&manager))
				printf("Error adding airport\n");
			break;

		case ePrintCompany:
			printCompany(&company,"Hachi","Babit","Ba","Olam",NULL);
			break;

		case ePrintAirports:
			printAirports(&manager);
			break;
		
		case ePrintFlightOrigDest:
			printFlightsCount(&company);
			break;
		
		case eSortFlights:
			sortFlight(&company);
			break;

		case eSearchFlight:
			findFlight(&company);
			break;

		case EXIT:
			printf("Bye bye\n");
			stop = 1;
			break;

		default:
			printf("Wrong option\n");
			break;
		}
	} while (!stop);


	saveManagerToFile(&manager, fileName);
	saveCompanyToFile(&company, binaryFileName);

	//uncomment to save company compressed to file
	//saveCompressedCompanyToFile(&company, binaryFileName);

	free(fileName);
	free(binaryFileName);
	freeManager(&manager);
	freeCompany(&company);

	system("pause");
	
}

int menu()
{
	int option;
	printf("\n\n");
	printf("Please choose one of the following options\n");
	for(int i = 0 ; i < eNofOptions ; i++)
		printf("%d - %s\n",i,str[i]);
	printf("%d - Quit\n", EXIT);
	scanf("%d", &option);
	
	//clean buffer
	char tav;
	scanf("%c", &tav);
	return option;
}

int initManagerAndCompany(AirportManager* pManager, char* managerFileName, Company* pCompany, char* companyFileName)
{
	if(managerFileName!= NULL)
	{
		int res = initManager(pManager, managerFileName);
		if (!res)
		{
			printf("error init manager\n");
			return 0;
		}
		if (res == FROM_FILE && companyFileName != NULL)
			return initCompanyFromFile(pCompany, pManager, companyFileName);
	}	
	else
		initCompany(pCompany, pManager);
	return 1;
}