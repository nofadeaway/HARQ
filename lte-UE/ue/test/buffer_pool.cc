#include "common/buffer_pool.h"
#include <stdio.h>

namespace srslte{

buffer_pool* buffer_pool::instance = NULL;
boost::mutex buffer_pool::instance_mutex;

buffer_pool* buffer_pool::get_instance(void)
{
  boost::lock_guard<boost::mutex> lock(instance_mutex);
  if(NULL == instance)
    instance = new buffer_pool();
  return instance;
}

void buffer_pool::cleanup(void)
{
  boost::lock_guard<boost::mutex> lock(instance_mutex);
  if(NULL != instance)
  {
    delete instance;
    instance = NULL;
  }
}

buffer_pool::buffer_pool()
{
  pool = new byte_buffer_t[POOL_SIZE];
  first_available = &pool[0];
  for(int i=0;i<POOL_SIZE-1;i++)
  {
    pool[i].set_next(&pool[i+1]);
  }
  pool[POOL_SIZE-1].set_next(NULL);
  allocated = 0;
}

byte_buffer_t* buffer_pool::allocate()
{
  boost::lock_guard<boost::mutex> lock(mutex);

  if(first_available == NULL)
  {
    printf("Error - buffer pool is empty\n");
    return NULL;
  }

  // Remove from available list
  byte_buffer_t* b = first_available;
  first_available = b->get_next();
  allocated++;

  return b;
}

void buffer_pool::deallocate(byte_buffer_t *b)
{
  boost::lock_guard<boost::mutex> lock(mutex);

  // Add to front of available list
  b->reset();
  b->set_next(first_available);
  first_available = b;
  allocated--;
}



} // namespace srsue
