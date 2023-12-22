
// data should be a pointer to a valid thread_data_t
// the lifetime shall be managed by the caller
int naive_reader_thread(void *data);
int naive_writer_thread(void *data);