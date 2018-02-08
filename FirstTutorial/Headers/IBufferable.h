//
//  IBufferable.h
//  FirstTutorial
//
//  Created by Kurt Bixby on 2/4/18.
//  Copyright © 2018 Bixby Productions. All rights reserved.
//

#ifndef IBufferable_h
#define IBufferable_h

class IBufferable {
public:
    virtual size_t DataSize() = 0;
    virtual size_t BufferData(GLuint offset) = 0;
};

#endif /* IBufferable_h */
