#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define n_stacks 7 
#define stack_cap 6

static int this_player ;
static int board[stack_cap + 1][n_stacks + 1] ;
static int top[n_stacks + 1] = {0, 1, 1, 1, 1, 1, 1, 1} ;

int const upward = 1 ;
int const downward = -1 ;
int const leftward = -1 ;
int const rightward = 1 ;

int _count (int stack, int level, int hdir, int vdir, int player)
{
	int count = 0 ;
	while (1 <= stack && stack <= n_stacks && 
	       1 <= level && level <= stack_cap) {
		if (board[level][stack] == player) {
			count++ ;
			stack += hdir ;
			level += vdir ;
		}
		else {
			break ;
		}
	}
	return count ; 
}

int count_down (int stack, int level, int player) 
{
	return _count(stack, level + downward, 0, downward, player) ;
}

int count_left (int stack, int level, int player) 
{
	return _count(stack + leftward, level, leftward, 0, player) ;
}

int count_right (int stack, int level, int player) 
{
	return _count(stack + rightward, level, rightward, 0, player) ;
}

int count_down_left (int stack, int level, int player) 
{
