#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include "MemoryManager.h"

using namespace std;


/// <summary>
/// Operators the specified out.
/// </summary>
/// <param name="out">The out.</param>
/// <param name="M">The m.</param>
/// <returns>out stream</returns>
ostream & operator<<(ostream & out, const MemoryManager &M)
{
   blocknode *tmp = M.firstBlock;
   assert(tmp);
   while(tmp)
   {
      out << "[" << tmp->bsize << ",";
      if (tmp->free)
	 out << "free] ";
      else
	 out << "allocated] ";
      if (tmp->next)
	 out << " -> "; 
      tmp = tmp->next;
   }
   return out;
}

/// <summary>
/// Initializes a new instance of the <see cref="MemoryManager"/> class.
/// </summary>
/// <param name="memtotal">The memtotal.</param>
MemoryManager::MemoryManager(unsigned int memtotal) : memsize(memtotal)
{
   baseptr = new unsigned char[memsize];
   firstBlock = new blocknode(memsize,baseptr);
}

blocknode *MemoryManager::getFirstPtr()
{
   return firstBlock;
}

/// <summary>
/// Mallocs the specified request.
/// </summary>
/// <param name="request">The request.</param>
/// <remarks> 
// Finds the first block in the list whose size is >= request
// If the block's size is strictly greater than request
// the block is split, with the newly create block being free. 
// It then changes the original block's free status to false
///</remarks>
/// <returns> pointer to a block of memory</returns>
unsigned char * MemoryManager::malloc(unsigned int request)
{
	//make a copy of the first block
	blocknode *head = this->firstBlock;

	//make the new node to be inserted
	blocknode *insertNode = new blocknode(head->bsize - request, nullptr, true);

	while (head)
	{
		if (head->free && head->bsize >= request)
		{
			if (head->bsize == request)
			{
				//doesn't need to be split so change the node to not be free and return the block of memory that it points to.
				head->free = false;
			}
			else
			{
				//this is the case we need to split the block into two
				splitBlock(head, request);
			}

			return head->bptr;
		}
		
		head = head->next;
	}
			
	return nullptr;
}

/// <summary>
/// Splits the block.
/// </summary>
/// <param name="p">The p.</param>
/// <param name="chunksize">The chunksize.</param>
/// <remarks>
// Utility function. Inserts a block after that represented by p
// changing p's blocksize to chunksize; the new successor node 
// will have blocksize the original blocksize of p minus chunksize and 
// will represent a free block.  
// Preconditions: p represents a free block with block size > chunksize
// and the modified target of p will still be free
///</remarks>
//
void MemoryManager::splitBlock(blocknode *p, unsigned int chunksize)
{
	//make sure the preconditions are true
	assert(p->bsize > chunksize);
	
	//insert node
	blocknode *insertNode = new blocknode((p->bsize - chunksize),nullptr,true, p);
	
	p->bsize = chunksize;
	p->free = false;
	p->next = insertNode;

	insertNode->bptr = p->bptr + p->bsize;

}

/// <summary>
/// Frees the specified blockptr.
/// </summary>
/// <remarks>
// makes the block represented by the blocknode free
// and merges with successor, if it is free; also 
// merges with the predecessor, it it is free
///</remarks>
/// <param name="blockptr">The blockptr.</param>
void MemoryManager::free(unsigned char *blockptr)
{
	blocknode *head = this->firstBlock;

	while (head)
	{
		if (head->bptr == blockptr)
		{
			head->free = true;
			
			//check to see if the node is the first node in the list which would have prev as null
			if (head->prev)
			{
				if (head->prev->free && head->next->free)
				{
					//case when the prev node and the next node are both free
					head = head->prev;
					mergeForward(head);
					mergeForward(head);
				}
				else if (head->prev->free && !head->next->free)
				{
					//case where only the prev node is free				 	
					head = head->prev;
					mergeForward(head);

					return;
				}
				else if (!head->prev->free && head->next->free)
				{
					//case where only next node is free
					mergeForward(head);

					return;
				}
			}

			return;
		}

		head = head->next;
	}
}

/// <summary>
/// Merges the forward.
/// </summary>
/// <remarks> 
///// merges two consecutive free blocks
// using a pointer to the first blocknode;
// following blocknode is deleted
///</remarks>
/// <param name="p">The p.</param>
void MemoryManager::mergeForward(blocknode *p)
{
	assert(p);
	assert(p->next);

	blocknode *temp = p->next;

	p->bsize += temp->bsize;
		
	if (temp->next)
	{
		p->next = temp->next;
		temp->next->prev = p;
	}
	else
	{
		p->next = nullptr;
	}

	//delete temp
	temp->bptr = nullptr;
	temp->next = nullptr;
	temp->prev = nullptr;
	
}

