// FIFO2.cpp
// Runs on any microcontroller
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size.  The file includes a transmit FIFO
// using index implementation and a receive FIFO using pointer
// implementation.  Other index or pointer implementation FIFOs can be
// created using the macros supplied at the end of the file.
// Created: 1/16/2020 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly


#include <stdint.h>
#include <stdlib.h>
#include "../inc/FIFO2.h"
#include "../inc/ST7735.h"


// A class named Queue that defines a FIFO
Queue::Queue(){
  // Constructor - set PutI and GetI as 0. 
  // We are assuming that for an empty Queue, both PutI and GetI will be equal
    // add code here to initialize on creation
    length = FIFOSIZE;
    counter = 0;
    head = NULL;
    tail = NULL;
}

// To check whether Queue is empty or not
bool Queue::IsEmpty(void){
    return (counter == 0);  // replace this with solution
}

  // To check whether Queue is full or not
bool Queue::IsFull(void){
    return (counter == length);  // replace this with solution
}

  // Inserts an element in queue at rear end
bool Queue::Put(char x){
    if(IsFull()) return false;
    counter++;
    Node* ptr = (Node *) malloc(sizeof(Node));
    ptr->data = x;
    ptr->next = NULL;
    if(counter == 1){
      head = ptr;
      tail = ptr;
      return true;
    }
    tail->next = ptr;
    tail = ptr;
    if(counter == 2){
      head->next = tail;
    }
    return true;
}

  // Removes an element in Queue from front end. 
bool Queue::Get(char* x){
    if(IsEmpty()) return false;
    counter--;
    *x = head->data;
    Node* temp = head->next;
    free(head);
    head = temp;
    if(counter == 0) tail = head; //tail = NULL
    return true;
}

  /* 
     Printing the elements in queue from front to rear. 
     This function is only to test the code. 
     This is not a standard function for Queue implementation. 
  */
void Queue::Print(void){
    // Finding number of elements in queue  
    // output to ST7735R
    Node* n = head;
    while(n != NULL){
      ST7735_OutChar(n->data);
      n = n->next;
    }
}

