#include <iostream>
#include <HttpCodec.h>

HttpCodec::HttpCodec(TransportDirection direction)
    : direction_(direction)
{
    // remember to free
    parser_ = (http_parser*) malloc(sizeof(http_parser)); 
    switch(direction){
    case TransportDirection::DOWNSTREAM:
        http_parser_init(parser_, HTTP_REQUEST);
        break;
    case TransportDirection::UPSTREAM:
        http_parser_init(parser_, HTTP_RESPONSE);
        break;
    default:
        std::cout << "Unknown direction" << std::endl;
    }

    parser_->data = this;
}

HttpCodec::~HttpCodec(){
    free(parser_);
    parser_ = nullptr;
}

void HttpCodec::setCallback(HttpSession* session){
    callback_ = session;
}

const http_parser_settings* HttpCodec::generateSettings(){
    static http_parser_settings settings;

    settings.on_message_begin = HttpCodec::onMessageBeginCB;
    settings.on_url = HttpCodec::onUrlCB;

    return &settings;
}

void HttpCodec::onIngress(char* buf, ssize_t recved){
    size_t nparsed;

    /* callback functions will be called after http_parser_execute() */
    nparsed = http_parser_execute(parser_, generateSettings(), buf, recved);
    dbPrint("nparsed= " << nparsed << std::endl);

}

int HttpCodec::onMessageBegin(){
    msg_.reset(new HttpMessage());
    
    callback_ -> onMessageBegin(msg_.get());
    return 0;
}

int HttpCodec::onUrl(const char* buf, size_t len){
    url_.append(buf);
    dbPrint("url is" << buf << std::endl);
    return 0;
}

int HttpCodec::onHeaderField(const char* buf, size_t len){}
int HttpCodec::onHeaderValue(const char* buf, size_t len){}
int HttpCodec::onHeadersComplete(){}
int HttpCodec::onBody(const char* buf, size_t len){}
int HttpCodec::onMessageComplete(){}

/* All callback functions must return 0 on success, return 1 on failure */
int HttpCodec::onMessageBeginCB(http_parser* parser){
    HttpCodec* codec = static_cast<HttpCodec*>(parser->data);

    try{
        return codec->onMessageBegin();
    }catch(const std::exception& ex){
        return 1;
    }
}

int HttpCodec::onUrlCB(http_parser* parser, const char* buf, size_t len){
    // static member function does not have "this" pointer
    // and cannot access non-static member function directly
    HttpCodec* codec = static_cast<HttpCodec*>(parser->data);
    
    try{
        return codec->onUrl(buf, len);
    }catch(const std::exception& ex){
        return 1;
    }
}

int HttpCodec::OnHeaderFieldCB(http_parser *parser, const char * buf, size_t len){}
int HttpCodec::OnHeaderValueCB(http_parser *parser, const char * buf, size_t len){}
int HttpCodec::OnHeadersCompleteCB(http_parser *parser){}
int HttpCodec::OnBodyCB(http_parser *parser, const char * buf, size_t len){}
int HttpCodec::OnMessageCompleteCB(http_parser *parser){}
     
