  class Exception
    : public std::exception
  {
    public:
      Exception(const std::string &msg = std::string()) : msg(msg) {}
      virtual ~Exception() throw() {}
      const char *what() const throw() { return msg.c_str(); }
    private:
      std::string msg;
  };

  class SqlExceptionC
    : public std::exception 
  {
    public:
      SqlExceptionC (std::string const& msg = std::string(), int code = -1) : m_msg (msg), m_code (code) {}
      virtual ~SqlExceptionC() throw() {}
      const char *what() const throw() { return m_msg.c_str(); }
      int code() const { return m_code; } 
    private:
      std::string m_msg;
      int m_code;
  };

#define EXCEPTION(EXCEPTION_NAME) \
    class EXCEPTION_NAME : public Exception \
    { \
      public: \
        EXCEPTION_NAME (std::string const& msg = std::string()) : Exception(msg) {} \
    };

#define SQL_EXCEPTION(EXCEPTION_NAME, c) \
    class EXCEPTION_NAME : public SqlExceptionC \
    { \
      public: \
        EXCEPTION_NAME (std::string const& msg = std::string(), int code = c) : SqlExceptionC (msg, c) {} \
    };
