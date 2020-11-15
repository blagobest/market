//
//  Interpreter.hpp
//  Market
//
//  Created by Blagovest on 06/11/20.
//

#ifndef Csv_Interpreter_hpp
#define Csv_Interpreter_hpp

#include <string>
#include <vector>

#include "Reader.hpp"

namespace mkt {
namespace csv {

template <typename Schema, char delim = ','>
class Interpreter {
    Reader<delim> reader;
    std::vector<std::string> values;
public:
    Interpreter(Reader<delim> & reader): reader(reader) {}
    
    bool get_next(Schema & row) {
        if (!reader.read_next_line(values)) {
            return false;
        }
        
        return row.from(values);
    }
    
    std::vector<std::string> get_headers() const {
        return reader.get_headers();
    }
};

}
}

#endif /* Interpreter_hpp */
