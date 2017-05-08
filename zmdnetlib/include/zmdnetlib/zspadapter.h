#ifndef ZSPADAPTER_H

#define ZSPADAPTER_H

class ZspAdapterImp;

class ZspAdapter
{
 public:
   ZspAdapter();
   ~ZspAdapter();
   void InitAdapter(int fd);
   int  HandleCmd(const char* msg, int msg_size);
   int  HandleIdle();
   void HandleClose();
 private:
   ZspAdapterImp* m_adapter;
};

#endif /* end of include guard: ZSPADAPTER_H */
