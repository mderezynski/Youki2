  class Exception : public std::exception
  {
    public:
      Exception(const std::string &msg = std::string()) : msg(msg) {}
      virtual ~Exception() throw() {}
      const char *what() const throw() { return msg.c_str(); }
    private:
      std::string msg;
  };

#define EXCEPTION(EXCEPTION_NAME) \
    class EXCEPTION_NAME : public Exception \
    { \
      public: \
        EXCEPTION_NAME (std::string const& msg = std::string()) : Exception(msg) {} \
    };
