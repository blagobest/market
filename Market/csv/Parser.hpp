//
//  Parser.hpp
//  Market
//
//  Created by Blagovest on 06/11/20.
//

#ifndef Csv_Parser_hpp
#define Csv_Parser_hpp

#include <iostream>
#include <string>

namespace mkt {
namespace csv {

template <char delim = ','>
class Parser {
protected:
    std::istream& in;
public:
    enum class Context { LINE_END, MIDDLE_VALUE, FILE_END };
    
    Parser(std::istream& in): in(in) {}
    
    Context get_next_token(std::string & token) noexcept {
        char c;
        token.clear();
        
        while (in.get(c)) {
            if (c == delim) {
                return Parser::Context::MIDDLE_VALUE;
            } else if (c == '\n') {
                return Parser::Context::LINE_END;
            } else {
                token.push_back(c);
            }
        }
        
        if (token.empty()) {
            return Parser::Context::FILE_END;
        } else {
            return Parser::Context::MIDDLE_VALUE;
        }
    }
    
};

}
}

#endif /* CSVParser_hpp */
