#define HALCC_EXCEPTION(EXCEPTION_NAME) \
    class EXCEPTION_NAME : public std::exception \
    { \
      public: \
        explicit EXCEPTION_NAME () : m_msg (std::string()) {} \
        explicit EXCEPTION_NAME (std::string const& msg) : m_msg (std::string(msg)) {} \
        const char *what() const throw() { return m_msg.c_str(); }  \
        virtual ~EXCEPTION_NAME() throw() {}  \
      private:  \
        std::string m_msg;  \
    };
