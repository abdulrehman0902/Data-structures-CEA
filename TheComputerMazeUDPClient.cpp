++++++++++++++++++++++++++++++++++++++++++++++// TheComputerMazeUDPClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <time.h>
#include <stdlib.h>

#pragma comment(lib, "wsock32.lib")

#define STUDENT_NUMBER		"12345678"
#define STUDENT_FIRSTNAME	"Fred"
#define STUDENT_FAMILYNAME	"Bloggs"

#define IP_ADDRESS_SERVER	"127.0.0.1"
//#define IP_ADDRESS_SERVER "164.11.80.69"


#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   5000
#define MAX_STRING_SIZE   200
#define MAX_NO_TOKENS     50

#define MAX_ITEMS_IN_ROOM		20
#define MAX_ITEMS_IN_BACKPACK	50


enum directions
{
	DIRECTION_NORTH = 0,
	DIRECTION_SOUTH = 1,
	DIRECTION_EAST = 2,
	DIRECTION_WEST = 3,
	DIRECTION_UP = 4,
	DIRECTION_DOWN = 5
};


enum direction_possible
{
	DIRECTION_NOT_PRESENT = -1,
	DIRECTION_LOCKED = 0,
	DIRECTION_OPEN = 1
};


enum item_types
{
	ITEM_NONE = 0,
	ITEM_BOOK = 1,
	ITEM_JUNK = 2,
	ITEM_EQUIPMENT = 3,
	ITEM_MANUSCRIPT = 4,
	ITEM_TEST = 5,
	ITEM_OTHER = 10
};



struct Item
{
	int  number;
	int  value;
	int  volume;
};


struct Student
{
	int level;
	int rooms_visited;
	int doors_openned;
	int number_of_moves;
	int score;
};


struct Room
{
	char name[5];
	int  type;
	int  direction[6];
	int  number_of_keys;
	int  keys[4];
	int  number_of_items;
	Item items[MAX_ITEMS_IN_ROOM];
};


struct Backpack
{
	int number_of_items;
	Item items[MAX_ITEMS_IN_BACKPACK];
};




#define MAX_OPTIONS	50

int number_of_options;
int options[MAX_OPTIONS];


Student student;
Room room;
Backpack backpack;



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer[MAX_BUFFER_SIZE];

char Tokens[MAX_NO_TOKENS][MAX_STRING_SIZE];

char text_student[1000];
char text_backpack[1000];
char text_room[1000];
char text_keys[1000];
char text_items[1000];
char text_options[1000];



void sentOption(int option, int key)
{
	char buffer[100];

	sprintf(buffer, "Option %d, %x", option, key);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}

#define OPTION_BASE_FOR_READS 200
#define OPTION_BASE_FOR_PICKUPS 500
#define OPTION_BASE_FOR_DROPS 800
#define OPTION_BASE_FOR_DOS 1100
#define OPTION_BASE_FOR_EVENTS 1300
#define OPTION_UNLOCK_NORTH 7
#define OPTION_UNLOCK_SOUTH 8
#define OPTION_UNLOCK_EAST 9
#define OPTION_UNLOCK_WEST 10
#define OPTION_MOVE_NORTH 1
#define OPTION_MOVE_SOUTH 2
#define OPTION_MOVE_EAST 3
#define OPTION_MOVE_WEST 4
#define OPTION_MOVE_UP 5
#define OPTION_MOVE_DOWN 6
/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

int option_count = 0;
char room_name[10] = " ";

int number_of_saved_keys = 0;
int saved_keys[2000];
int last_direction = DIRECTION_NORTH;

int keys_stored[2000];//this array will store the keys that we have picked up
int ns;
int ew;
int floor;
char rooms[2000000];// this will store the visited rooms 
int room_visited_counter = 0;
int room_iterator = 0;
int first_index_counter = 0;
int second_index_counter = 0;
int third_index_counter = 0;
int fourth_index_counter = 0;
int try_key = 0;
int trykey = -1;
int i = 0;
int pickedup_keys[1000];


//bool rooms_visited[5][10][10];

/*
bool room_visited() {
	int floor;
	int ns;
	int ew;
	for (floor = 0; floor < 5; floor++) {
		for (ns = 0; ns < 10; ns++) {
			for (ew = 0; ew < 5; ew++) {
				rooms_visited[floor][ns][ew] = false;
			}
		}
	}
	return 0;
}*/

int priority()
{
	int num = rand() % 6 + 1;
	return num;
}

void pickUp() {
	for (int i = 0; i < room.number_of_items; i++) {
		sentOption(OPTION_BASE_FOR_PICKUPS + room.items[i].number, 0);
		backpack.items[i] = room.items[i];
	}
}

// this function will tell us if the room we are entering is already visited or not
bool room_visited(Room r) {
	while (room_visited_counter <= 3)
	{
		try {
		for (int i = 0; i < 2000; i = i + 1) {
			if (r.name[0] == rooms[first_index_counter]) {
				room_visited_counter = room_visited_counter + 1;
			}
			else {
				first_index_counter = first_index_counter + 4;
				room_visited_counter = 0;
			}
			if (r.name[1] == rooms[second_index_counter]) {
				room_visited_counter = room_visited_counter + 1;
			}
			else {
				second_index_counter = second_index_counter + 4;
				room_visited_counter = 0;
			}
			if (r.name[2] == rooms[third_index_counter]) {
				room_visited_counter = room_visited_counter + 1;
			}
			else {
				third_index_counter = third_index_counter + 4;
				room_visited_counter = 0;
			}

		}
		}
		catch (...) {}
	}
	if (room_visited_counter == 3) {
		return true; // return true when room visited
	}
	else {
		return false;	// return false when new room
	}
} 

//checking if given key is already present in
bool key_already_saved(int new_key)
{
	int i;
	if (number_of_saved_keys > 0)
	{
		for (i = 0; i < number_of_saved_keys; i++)
		{
			if (new_key == saved_keys[i])
				return true;
		}
	}
	return false;
}

void do_work() {
	for (int i = 0; i < backpack.number_of_items; i++) {
		if (backpack.items[i].number == ITEM_OTHER) {
			sentOption(OPTION_BASE_FOR_DOS + backpack.items[i].number, 0);
		}
		else if (backpack.items[i].number == ITEM_EQUIPMENT) {
			sentOption(OPTION_BASE_FOR_DOS + backpack.items[i].number, 0);
		}
		else if (backpack.items[i].number == ITEM_JUNK) {
			sentOption(OPTION_BASE_FOR_DOS + backpack.items[i].number, 0);
		}
		else if (backpack.items[i].number == ITEM_MANUSCRIPT) {
			sentOption(OPTION_BASE_FOR_DOS + backpack.items[i].number, 0);
		}
		else if (backpack.items[i].number == ITEM_TEST) {
			sentOption(OPTION_BASE_FOR_DOS + backpack.items[i].number, 0);
		}
		else if (backpack.items[i].number == ITEM_BOOK) {
			sentOption(OPTION_BASE_FOR_READS + backpack.items[i].number, 0);
		}
	}
}

void yourMove()
{

	printf("%d", priority());
	printf("--------------------------------------------------------------------------------------------------\n");

	switch (priority())
	{
	case 1: {
		if (room.direction[OPTION_MOVE_WEST] == DIRECTION_OPEN && !room_visited(room)) { // WILL CHECK IF THE NORTH IS OPEN WILL MOVE TO NORTH

			last_direction = room.direction[DIRECTION_WEST];
			sentOption(OPTION_MOVE_WEST, 0); // DOOR OPEN SO NOT APPLYING KEYS
			printf("You have moved west");
			if (room.number_of_items > 0 && room.items[0].value > 0) {
				//sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				//backpack.number_of_items = room.items[0].number;
				pickUp();
				do_work();
			}

			//collecting keys and checking if they are already not present
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i <= room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
			if (i >= 4) {
				i = i % 4;
			}
			rooms[room_iterator] = room.name[i];//storing this room in array
			room_iterator += 4; //moving ahead 4 as the room name is 4 characters long
			break;
		}

	}
	case 2: {
		if (room.direction[OPTION_MOVE_EAST] == DIRECTION_OPEN && !room_visited(room))
		{
			last_direction = DIRECTION_EAST;
			sentOption(OPTION_MOVE_EAST, 0); // DOOR OPEN SO NOT APPLYING KEYS
			printf("You have moved east");
			if (room.number_of_items > 0 && room.items[0].value > 0) {
				//sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				//backpack.number_of_items = room.items[0].number;
				pickUp();
				do_work();
			}
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
			if (i >= 4) {
				i = i % 4;
			}
			rooms[room_iterator] = room.name[i];
			room_iterator += 4;
			//THIS WILL PICK UP THE OBJECT IN ROOM
			break;
		}

	}
	case 3: {
		if (room.direction[OPTION_MOVE_NORTH] == DIRECTION_OPEN && !room_visited(room)) {

			last_direction = DIRECTION_NORTH;
			sentOption(OPTION_MOVE_NORTH, 0); // DOOR OPEN SO NOT APPLYING KEYS
			printf("You have moved north");
			if (room.number_of_items > 0 && room.items[0].value > 0) {
				//sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				//backpack.number_of_items = room.items[0].number;
				pickUp();
				do_work();
			}
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
			if (i >= 4) {
				i = i % 4;
			}
			rooms[room_iterator] = room.name[i];
			room_iterator += 4;

			break;
		}

	}
	case 4: {
		if (room.direction[OPTION_MOVE_SOUTH] == DIRECTION_OPEN && !room_visited(room)) {

			sentOption(OPTION_MOVE_SOUTH, 0); // DOOR OPEN SO NOT APPLYING KEYS
			printf("You have moved south");
			if (room.number_of_items > 0 && room.items->value > 0) {
				//sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				//backpack.number_of_items = room.items[0].number;
				pickUp();
				do_work();
			}
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
			if (i >= 4) {
				i = i % 4;
			}
			rooms[room_iterator] = room.name[i];
			room_iterator += 4;
			last_direction = DIRECTION_SOUTH;
			break;
		}

	}
	case 5: {
		if (room.direction[OPTION_MOVE_UP] == DIRECTION_OPEN) {

			sentOption(OPTION_MOVE_UP, 0); // DOOR OPEN SO NOT APPLYING KEYS
			printf("You have moved up");
			if (room.number_of_items > 0 && room.items->value > 0) {
				//sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				//backpack.number_of_items = room.items[0].number;
				pickUp();
				do_work();
			}
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
			if (i >= 4) {
				i = i % 4;
			}
			rooms[room_iterator] = room.name[i];
			room_iterator += 4;
			break;
		}

	}
	case 6: {
		if (room.direction[OPTION_MOVE_DOWN] == DIRECTION_OPEN) {
			sentOption(OPTION_MOVE_DOWN, 0); // DOOR OPEN SO NOT APPLYING KEYS
			printf("You have moved down");
			if (room.number_of_items > 0 && room.items->value > 0) {
				//sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				//backpack.number_of_items = room.items[0].number;
				pickUp();
				do_work();
			}
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}


			if (i >= 4) {
				i = i % 4;
			}
			rooms[room_iterator] = room.name[i];
			room_iterator += 4;
			break;
		}

	}
	default: {
		printf("Entered default \n");
		if ((room.direction[OPTION_MOVE_NORTH] == DIRECTION_LOCKED) && (number_of_saved_keys > 0) && (number_of_saved_keys > try_key)) {
			sentOption(OPTION_UNLOCK_NORTH, saved_keys[try_key]);
			try_key++;
			printf("Door opened");

			if (room.number_of_items > 0 && room.items->value > 0) {
				sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				backpack.number_of_items = room.items[0].number;
			}
			//collecting keys and checking if they are already not present
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
		}
		else if ((room.direction[OPTION_MOVE_SOUTH] == DIRECTION_LOCKED) && (number_of_saved_keys > 0) && (number_of_saved_keys > try_key)) {
			sentOption(OPTION_UNLOCK_SOUTH, saved_keys[try_key]);
			try_key++;
			printf("Door opened");
			if (room.number_of_items > 0 && room.items->value > 0) {
				sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				backpack.number_of_items = room.items[0].number;
			}

			//colleting keys and checking if they are already not present
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
		}
		else if ((room.direction[OPTION_MOVE_WEST] == DIRECTION_LOCKED) && (number_of_saved_keys > 0) && (number_of_saved_keys > try_key)) {
			sentOption(OPTION_UNLOCK_WEST, saved_keys[try_key]);
			try_key++;
			printf("Door opened");
			if (room.number_of_items > 0 && room.items->value > 0) {
				sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				backpack.number_of_items = room.items[0].number;
			}

			//collecting keys and checking if they are already not present
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
		}
		else if ((room.direction[OPTION_MOVE_EAST] == DIRECTION_LOCKED) && (number_of_saved_keys > 0) && (number_of_saved_keys > try_key)) {
			sentOption(OPTION_UNLOCK_EAST, saved_keys[try_key]);
			try_key++;
			printf("Door opened");
			if (room.number_of_items > 0 && room.items->value > 0) {
				sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				backpack.number_of_items = room.items[0].number;
			}

			//collecting keys and checking if they are already not present
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
		}
		else {
			sentOption(OPTION_MOVE_WEST, 0); // DOOR OPEN SO NOT APPLYING KEYS
			if (room.number_of_items > 0 && room.items->value > 0) {
				sentOption(OPTION_BASE_FOR_PICKUPS + room.items[0].number, 0);
				backpack.number_of_items = room.items[0].number;
			}
			//collecting keys and checking if they are already not present
			if (room.number_of_keys > 0)
			{
				for (int i = 0; i < room.number_of_keys; i++)
				{
					if (!key_already_saved(room.keys[i]))
					{
						saved_keys[number_of_saved_keys] = room.keys[i];
						number_of_saved_keys++;
					}
				}
			}
			if (i >= 4) {
				i = i % 4;
			}
			try {
				rooms[room_iterator] = room.name[i];//storing this room in array
				room_iterator += 4; //moving ahead 4 as the room name is 4 characters long

			}
			catch(...){

			}

		}

	}
	}

	//pre-written
	printf("we have saved %d keys \n\n", number_of_saved_keys);
	if (strcmp(room_name, room.name) != 0) option_count = 0;
	sentOption(options[option_count], 0x1234);
	option_count = (option_count + 1) % number_of_options;
}

/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/




int getTokens(char* instring, char seperator)
{
	int  number_of_tokens;
	char chr;
	int  state;
	int  i;
	int  j;


	for (i = 0; i < MAX_NO_TOKENS; i++)
	{
		for (j = 0; j < MAX_STRING_SIZE; j++)
		{
			Tokens[i][j] = '\0';
		}
	}

	number_of_tokens = -1;
	chr = instring[0];
	state = 0;
	i = 0;

	while (chr != '\0')
	{
		switch (state)
		{
		case 0:  // Initial state
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 1:  // Leading white space
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 2:  // Collecting chars
			if (chr == seperator)
			{
				//number_of_tokens++;
				state = 1;
			}
			else
			{
				j++;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		default:
			break;
		}

		i++;
		chr = instring[i];
	}

	return (number_of_tokens + 1);
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}



void printRoom()
{
	int i;

	printf("Room\n");
	printf("Room = %s, Room type = %d\n", room.name, room.type);

	printf("Directions = ");
	for (i = 0; i < 6; i++)
	{
		printf("%d  ", room.direction[i]);
	}
	printf("\n");

	if (room.number_of_keys > 0)
	{
		printf("Keys = ");
		for (i = 0; i < room.number_of_keys; i++)
		{
			printf("0x%X  ", room.keys[i]);
		}
		printf("\n");
	}
	else
	{
		printf("No keys in this room\n");
	}

	if (room.number_of_items > 0)
	{
		for (i = 0; i < room.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", room.items[i].number, room.items[i].value, room.items[i].volume);
		}
	}
	else
	{
		printf("No items in this room\n");
	}

	printf("\n");
}


void printStudent()
{
	printf("Student\n");
	printf("Level=%d,  Number of rooms visited = %d,  Number of doors openned = %d,  Number of moves = %d,  Score = %d\n", student.level, student.rooms_visited, student.doors_openned, student.number_of_moves, student.score);
	printf("\n");
}


void printBackpack()
{
	int i;

	printf("Backpack\n");

	if (backpack.number_of_items > 0)
	{
		for (i = 0; i < backpack.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", backpack.items[i].number, backpack.items[i].value, backpack.items[i].volume);
		}
	}
	else
	{
		printf("Your backpack is empty\n");
	}
	printf("\n");
}


void printOptions()
{
	int i;

	printf("Options\n");
	printf("Options = ");
	for (i = 0; i < number_of_options; i++)
	{
		printf("%d  ", options[i]);
	}
	printf("\n");
	printf("\n");
}




void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	int  i;
	char* p;
	int	 number_of_tokens;


	sprintf_s(buffer, "Register  %s %s %s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (true)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			p = ::inet_ntoa(client_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				printf("%s\n\n", buffer);

				number_of_tokens = getTokens(buffer, '|');

				if (number_of_tokens == 6)
				{
					strcpy(text_student, Tokens[0]);
					strcpy(text_backpack, Tokens[1]);
					strcpy(text_room, Tokens[2]);
					strcpy(text_keys, Tokens[3]);
					strcpy(text_items, Tokens[4]);
					strcpy(text_options, Tokens[5]);

					printf("Student  = '%s'\n", text_student);
					printf("Backpack = '%s'\n", text_backpack);
					printf("Room     = '%s'\n", text_room);
					printf("Keys     = '%s'\n", text_keys);
					printf("Items    = '%s'\n", text_items);
					printf("Options  = '%s'\n", text_options);

					student.level = -1;
					student.rooms_visited = -1;
					student.doors_openned = -1;
					student.number_of_moves = -1;

					if (sscanf(text_student, "%d,%d,%d,%d,%d", &student.level, &student.rooms_visited, &student.doors_openned, &student.number_of_moves, &student.score) == 5)
					{
					}

					if (strlen(text_backpack) > 0)
					{
						backpack.number_of_items = getTokens(text_backpack, '&');

						if (backpack.number_of_items > 0)
						{
							for (i = 0; i < backpack.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_BACKPACK)
								{
									backpack.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &backpack.items[i].number, &backpack.items[i].value, &backpack.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						backpack.number_of_items = 0;
					}

					sscanf(text_room, "%s ,%d, %d, %d, %d, %d, %d, %d", &room.name, &room.type, &room.direction[DIRECTION_NORTH], &room.direction[DIRECTION_SOUTH], &room.direction[DIRECTION_EAST], &room.direction[DIRECTION_WEST], &room.direction[DIRECTION_UP], &room.direction[DIRECTION_DOWN]);

					if (strlen(text_keys) > 0)
					{
						room.number_of_keys = getTokens(text_keys, '&');

						if (room.number_of_keys > 0)
						{
							for (i = 0; i < room.number_of_keys; i++)
							{
								if (i < 4)
								{
									room.keys[i] = -1;

									if (sscanf(Tokens[i], "%x", &room.keys[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_keys = 0;
					}

					if (strlen(text_items) > 0)
					{
						room.number_of_items = getTokens(text_items, '&');

						if (room.number_of_items > 0)
						{
							for (i = 0; i < room.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_ROOM)
								{
									room.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &room.items[i].number, &room.items[i].value, &room.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_items = 0;
					}

					if (strlen(text_options) > 0)
					{
						number_of_options = getTokens(text_options, ',');

						if (number_of_options > 0)
						{
							for (i = 0; i < number_of_options; i++)
							{
								if (i < MAX_OPTIONS)
								{
									options[i] = -1;

									if (sscanf(Tokens[i], "%d", &options[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						number_of_options = 0;
					}
				}

				printStudent();
				printBackpack();
				printRoom();
				printOptions();

				system("timeout /t 60");

				yourMove();
			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}




int main()
{
	char chr = '\0';

	printf("\n");
	printf("The Computer Maze Student Program\n");
	printf("UWE Computer and Network Systems Assignment 2 \n");
	printf("\n");

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) 
	{
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	////	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//	printf("Bind failed! %d\n", WSAGetLastError());
	//}

	communicate_with_server();

	closesocket(sock);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}

