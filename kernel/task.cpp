#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>

Task::Task() {
    debugf( "Task constructor." );
}

Task::~Task() {
    debugf( "Task deconstructor." );
}

void Task::open( void ) {

}

void Task::close( void ) {

}

void Task::read( void ) {

}

void Task::write( void ) {

}

void task_initalize( void ) {
    Task    *kernel_task;
}