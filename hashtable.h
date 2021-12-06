#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "helper.h"

#define TABLE_SIZE 67

typedef struct {
		int fd;
		clientConnData *cliInfo;
		bool present;
} connection;

typedef connection connectionTable[TABLE_SIZE];

void initTable(connectionTable table);

int hash(int fd);

clientConnData *searchTable(connectionTable table, int fd);

void insertInTable(connectionTable table, int fd, clientConnData *clInfo);

void removeFromTable(connectionTable table, int fd);
#endif