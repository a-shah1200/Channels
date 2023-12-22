#include "channel.h"
#include "linked_list.h"

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
chan_t* channel_create(size_t size){  
    /* IMPLEMENT THIS */
    /*Currently assuming size > 0 i.e. buffered case*/
    chan_t* ch = (chan_t*) malloc(sizeof(chan_t));
    ch->buffer=buffer_create(size);
    ch->open=1;
    //list_create();
    ch->receive_ops = list_create();
    ch->send_ops = list_create();
    pthread_mutex_init(&ch->lock,NULL);
    pthread_mutex_init(&ch->select_lock,NULL);
    //pthread_mutex_init(&ch->send_lock,NULL);
    //pthread_mutex_init(&ch->receive_lock,NULL);
    pthread_cond_init(&ch->full,NULL);
    pthread_cond_init(&ch->empty,NULL);
    return ch;
}

// Writes data to the given channel
// This can be both a blocking call i.e., the function only returns on a successful completion of send (blocking = true), and
// a non-blocking call i.e., the function simply returns if the channel is full (blocking = false)
// In case of the blocking call when the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// WOULDBLOCK if the channel is full and the data was not added to the buffer (non-blocking calls only),
// CLOSED_ERROR if the channel is closed, and
// OTHER_ERROR on encountering any other generic error of any sort
enum chan_status channel_send(chan_t* channel, void* data, bool blocking){    
    pthread_mutex_lock(&channel->lock);
    if (channel->open==0){
        pthread_mutex_unlock(&channel->lock);
        return CLOSED_ERROR;
    }
    
    buffer_t* buff = channel->buffer;
    if (blocking){
        while ((buff->size >= buff->capacity) && (channel->open==1)){ 
            pthread_cond_wait(&channel->full,&channel->lock);
        }
        if (channel->open==0){
            pthread_mutex_unlock(&channel->lock);
            return CLOSED_ERROR;
        }
        buffer_add(data,channel->buffer);
        pthread_mutex_lock(&channel->select_lock);
        list_node_t* node=list_begin(channel->send_ops);
        while (node!=NULL){
            sem_post(node->data);
            node=list_next(node);
            }
        pthread_mutex_unlock(&channel->select_lock);

        pthread_cond_signal(&(channel->empty));
        pthread_mutex_unlock(&(channel->lock));
        return SUCCESS;
        /*

        if (buff->size < buff->capacity){
            buffer_add(data,channel->buffer);
            pthread_cond_signal(&(channel->empty));
            pthread_mutex_unlock(&(channel->lock));
            return SUCCESS;
        }
        else{
            printf("Something is wrong, with adding data to buffer\n");
            pthread_cond_signal(&(channel->empty));
            pthread_mutex_unlock(&(channel->lock));
            return OTHER_ERROR;
        }*/
    }
    else{
        
        if(buff->size >= buff->capacity){
            pthread_mutex_unlock(&channel->lock);
            return WOULDBLOCK;
        }
        else{
            buffer_add(data,channel->buffer);
            pthread_mutex_lock(&channel->select_lock);
            list_node_t* node=list_begin(channel->send_ops);
            while (node!=NULL){
                sem_post(node->data);
                node=list_next(node);
            }
            
            pthread_mutex_unlock(&channel->select_lock);
            pthread_cond_signal(&(channel->empty));
            pthread_mutex_unlock(&(channel->lock));
            return SUCCESS;
            /*
            if (buff->size < buff->capacity){
                buffer_add(data,channel->buffer);
                pthread_cond_signal(&(channel->empty));
                pthread_mutex_unlock(&(channel->lock));
                return SUCCESS;
            }
            else{
                printf("Something is wrong, with adding data to buffer\n");
                pthread_cond_signal(&(channel->empty));
                pthread_mutex_unlock(&(channel->lock));
                return OTHER_ERROR;
            }*/

        }
    }
}

// Reads data from the given channel and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This can be both a blocking call i.e., the function only returns on a successful completion of receive (blocking = true), and
// a non-blocking call i.e., the function simply returns if the channel is empty (blocking = false)
// In case of the blocking call when the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// WOULDBLOCK if the channel is empty and nothing was stored in data (non-blocking calls only),
// CLOSED_ERROR if the channel is closed, and
// OTHER_ERROR on encountering any other generic error of any sort
enum chan_status channel_receive(chan_t* channel, void** data, bool blocking){   
    pthread_mutex_lock(&channel->lock);
    
    if (channel->open == 0){
        pthread_mutex_unlock(&channel->lock);
        return CLOSED_ERROR;
    }
    buffer_t* buff=channel->buffer;
    if (blocking){
        while ((buff->size <= 0)&&(channel->open==1)){
            pthread_cond_wait(&channel->empty,&channel->lock);
        }
        if (channel->open==0){
            pthread_mutex_unlock(&channel->lock);
            return CLOSED_ERROR;
        }
        
        *data=buffer_remove(channel->buffer);
        pthread_mutex_lock(&channel->select_lock);
        list_node_t* node=list_begin(channel->receive_ops);
        while (node!=NULL){
            sem_post(node->data);
            node=list_next(node);
            }
        pthread_mutex_unlock(&channel->select_lock);

        pthread_cond_signal(&channel->full);
        pthread_mutex_unlock(&channel->lock);
        return SUCCESS;
    }
    else{
        if (buff->size <= 0){
            pthread_mutex_unlock(&channel->lock);
            return WOULDBLOCK;
        }
        else{
            *data=buffer_remove(buff);
            pthread_mutex_lock(&channel->select_lock);
            list_node_t* node=list_begin(channel->receive_ops);
            while (node!=NULL){
                sem_post(node->data);
                node=list_next(node);
            }
            
            pthread_mutex_unlock(&channel->select_lock);
            pthread_cond_signal(&channel->full);
            pthread_mutex_unlock(&channel->lock);
            return SUCCESS;
            }
        }
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// OTHER_ERROR in any other error case
enum chan_status channel_close(chan_t* channel){
    pthread_mutex_lock(&channel->lock);
    if (channel->open == 0){
        pthread_mutex_unlock(&channel->lock);
        return CLOSED_ERROR;
    }
    else{
        channel->open = 0;
        pthread_mutex_lock(&channel->select_lock);
        list_node_t* node_s=list_begin(channel->send_ops);
        while(node_s!=NULL){
            sem_post(node_s->data);
            node_s=list_next(node_s);
        }
        list_node_t* node_r=list_begin(channel->receive_ops);
        while(node_r!=NULL){
            sem_post(node_r->data);
            node_r=list_next(node_r);
        }
    
        pthread_mutex_unlock(&channel->select_lock);
        pthread_cond_broadcast(&channel->full);
        pthread_cond_broadcast(&channel->empty);
    }
    pthread_mutex_unlock(&channel->lock);
    return SUCCESS;
    /*//Check if close is successful
    if(pthread_cond_signal(&channel->full) != 0){
        return OTHER_ERROR;
    }
    if(pthread_cond_signal(&channel->empty) != 0){
        return OTHER_ERROR;
    }
    if(pthread_mutex_unlock(&channel->lock) != 0){
        return OTHER_ERROR;
    }
    channel->open = 0;
    return SUCCESS;*/
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// OTHER_ERROR in any other error case
enum chan_status channel_destroy(chan_t* channel){
    /* IMPLEMENT THIS */
    if (channel->open == 1){
        return(DESTROY_ERROR);
    }
    buffer_free(channel->buffer);
    pthread_mutex_destroy(&channel->lock);
    pthread_cond_destroy(&channel->full);
    pthread_cond_destroy(&channel->empty);
    pthread_mutex_destroy(&channel->select_lock);
    list_destroy(channel->send_ops);
    list_destroy(channel->receive_ops);
 
    free(channel);
    
    return SUCCESS;
}

// Takes an array of channels, channel_list, of type select_t and the array length, channel_count, as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error





enum chan_status channel_select(size_t channel_count, select_t* channel_list, size_t* selected_index){

    select_t* op;
    //select_t* ops[channel_count];
    int op_outcome = 0;
    sem_t semaphore;
    sem_init(&semaphore,0,0);




    for (int i=0;i<channel_count;i++){
        op=channel_list+i;
        pthread_mutex_lock(&op->channel->select_lock);
        if (op->is_send){
            list_insert(op->channel->receive_ops,&semaphore);
        }
        else{
            list_insert(op->channel->send_ops,&semaphore);
        }
        pthread_mutex_unlock(&op->channel->select_lock);
        }


    while (true){
    op=channel_list;

    for (size_t i=0;i<channel_count;i++){

        op=channel_list+i;
        if (op->is_send){
            op_outcome=channel_send(op->channel,op->data,false);
        }
        else{
            op_outcome=channel_receive(op->channel,&op->data,false);
           
        }
        if ((op_outcome==OTHER_ERROR)||(op_outcome==CLOSED_ERROR)||(op_outcome==SUCCESS))
        {
            /*if ((op_outcome==SUCCESS)&&(op->is_send!=true)){
                pthread_mutex_lock(&op->channel->lock);
                op->data=op->channel->buffer->data[i];
                pthread_mutex_unlock(&op->channel->lock);

            }*/
            *selected_index=i;
            goto final;
        }
    }
    sem_wait(&semaphore);
    }

    final:
        op=channel_list;
        for (int i=0;i<channel_count;i++){
            op=channel_list+i;
            if (op->is_send){
                pthread_mutex_lock(&op->channel->select_lock);
                list_remove(op->channel->receive_ops,list_find(op->channel->receive_ops,&semaphore));
                pthread_mutex_unlock(&op->channel->select_lock);
            }
            else{
                pthread_mutex_lock(&op->channel->select_lock);
                list_remove(op->channel->send_ops,list_find(op->channel->send_ops,&semaphore));
                pthread_mutex_unlock(&op->channel->select_lock);   
            }
        }
        sem_destroy(&semaphore);
        return op_outcome;




}








