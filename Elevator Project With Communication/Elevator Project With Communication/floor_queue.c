#include "floor_queue.h"

//static int16_t queue_data[FLOOR_QUEUE_SIZE]; //This creates the actual array that stores floor numbers.
//static means these variables are private to this file
static uint8_t queue_front = 0;//This points to the first item in the queue.
static uint8_t queue_rear = 0;//This points to the next free place where a new floor will be added

static uint8_t queue_count = 0;//This tells how many floors are currently stored.

void floor_queue_init(void)
{
    queue_front = 0;
    queue_rear = 0;
    queue_count = 0;
}

uint8_t floor_queue_is_empty(void)
{
    return (queue_count == 0);//if there is 0 items, return true values
}

uint8_t floor_queue_is_full(void)
{
    return (queue_count >= 10); //if queue already has the maximum number of requests
}

uint8_t floor_queue_contains(int16_t floor)
{
    uint16_t queue_data[10]; // defining the storage array
    uint8_t i;//i is loop counter
    uint8_t index;// index is position inside array

    for (i = 0; i < queue_count; i++)// starts from first item, runs as many times mentioned in count
    {
        index = (queue_front + i) % 10;//starts from front and move forward by 1; come backt to 0 if the system reaches the end of  array
        //floor_queue_size reuse old empty spaces.
        if (queue_data[index] == floor)//this compare stored floor with wanted floor
        {
            return 1;
        }
    }
    return 0;
}

uint8_t floor_queue_enqueue(int16_t floor) //add a floor to queue
{
    uint16_t queue_data[10]; // defining the storage array
    if (floor_queue_is_full())
    {
        return 0;
    }

    queue_data[queue_rear] = floor;//put new floor in current rear position
    queue_rear = (queue_rear + 1) % 10;//same logic as before (10 is the max size of the storage)
    queue_count++;

    return 1;
}

uint8_t floor_queue_dequeue(int16_t *floor){//creating FIFO
    uint16_t queue_data[10]; // defining the storage array
    if (floor_queue_is_empty())//if nothing is stored it wont remove anything
    {
        return 0;
    }

    *floor = queue_data[queue_front];//next floor
    queue_front = (queue_front + 1) % 10;//next item becomes the first item
    queue_count--;//count goes down

    return 1;
}

void floor_queue_clear(void)
{
    queue_front = 0;
    queue_rear = 0;
    queue_count = 0;
}

void add_floor(int16_t arr[], int *size, int16_t floor_input) {
    arr[*size] = floor_input;  
    (*size)++;   
    print_array(arr,size);             
}

void delete_first_floor(int16_t arr[], int *size){
    for(int i = 0; i < *size - 1;i++){
        arr[i] = arr[i + 1];
    }
    (*size)--;
}

uint8_t array_empty(int16_t arr[],int *size){
    return *size == 0;
}


int16_t first_array_floor(int16_t arr[], int *size){
    if (!array_empty(arr,size)){
        return arr[0];
    }
    return arr[0]; // it was -1 before
}

void clear_array(int16_t arr[], int *size) {
    for (int i = 0; i < *size; i++) {
        arr[i] = 0;  // set every element to 0
    }
    (*size) = 0;  // reset size to 0
}

void print_array(int16_t arr[], int *size) {
    for (int i = 0; i < *size; i++) {
        printf("%d,", arr[i]);
    }
}