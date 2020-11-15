//
//  Reader.hpp
//  Market
//
//  Created by Blagovest on 06/11/20.
//

#ifndef Csv_Reader_hpp
#define Csv_Reader_hpp

#include <iostream>
#include <vector>
#include <string>

#include "Parser.hpp"

namespace mkt {
namespace csv {

template <char delim = ','>
class Reader {
    Parser<delim> parser;
    std::vector<std::string> headers;
    
public:
    Reader(std::istream& in): parser(in), headers() {
        // first line is assumed to be the headers
        read_next_line(headers);
    }
    
    bool read_next_line(std::vector<std::string> & values) noexcept {
        std::string token;
        values.clear();
        typename Parser<delim>::Context context;
        
        while (true) {
            context = parser.get_next_token(token);
            
            if (!token.empty()) {
                values.push_back(token);
            }
            
            if (context != Parser<delim>::Context::MIDDLE_VALUE) {
                break;
            }
        }
        
        return !values.empty();
    }
    const std::vector<std::string> & get_headers() const noexcept { return headers; }
};
    
}
}

#endif /* Reader_hpp */
