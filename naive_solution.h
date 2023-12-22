
// data should be a pointer to a valid thread_data_t
// the lifetime shall be managed by the caller
int reader_thread(void *data);
int writer_thread(void *data);