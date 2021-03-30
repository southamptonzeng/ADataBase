#include <iostream>
#include <time.h>
#include "CLStatus.h"
#include "CLTable.h"

#define IsShow true
#define NotShow false

#define IsShowBTree true
#define NotShowBTree false
/*
int main()
{
	struct Table t;
	
	srand((unsigned)time(NULL));
	t=CLTable::CreateRandTableMsg(IsShow,1,10000);
	CLTable::WriteTableMsg(&t);
	
	return 0;
}
*/
/*
int main()
{
	CLTable::ReadOneTableMsg(0);
	return 0;
}
*/

int main()
{
	struct Table t;
	srand((unsigned)time(NULL));
	for(int i=0;i<100;i++)
	{
		t=CLTable::CreateRandTableMsg(NotShow,1,10000);
		CLTable::WriteTableMsg(&t);
	}
	CLTable::SearchTableMsg(0,200,1000);
	return 0;
}

/*
int main()
{
	struct Table t;
	srand((unsigned)time(NULL));
	for(int i=0;i<15;i++)
	{
		t=CLTable::CreateRandTableMsg(NotShow,1,10000);
		CLTable::WriteTableMsg(&t);
	}
	CLTable::ShowBTree();
	return 0;
}
*/
/*
int main()
{
	struct Table t;
	srand((unsigned)time(NULL));
	for(int i=0;i<15;i++)
	{
		t=CLTable::CreateRandTableMsg(NotShow,1,10000);
		CLTable::WriteTableMsg(&t);
		CLTable::ShowBTree();
	}
	CLTable::SaveBTreeMsg();
	return 0;
}
*/
/*
int main()
{
	CLTable::ReadBTreeMsg();
	CLTable::ShowBTree();
	return 0;
}
*/
//t=CLTable::CreateRandTableMsg(IsShow,1,10000);
//CLTable::WriteTableMsg(&t);
//CLTable::ReadOneTableMsg(0);
//CLTable::SearchTableMsg(0,1,9);