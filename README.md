# Principles of Embedded Software Engineering

The principles mentioned in this document are my personal learnings I made when working on the Rescue Bee Hive UAV Hangar as the main developer aswell as working on my personal projects. 
They are supposed to help me increase my understanding of each problem and suit as a reference for the future.  

## Code Quality

The following principles have been brought up by using the `-Wall`-flag on the gcc and treating them as errors.

### Shadowing [-werror=shadow]

Using the same name for variables in different scopes is not good practice for the following reasons:
- reduced readability
- harder debugging

To resolve that issue using a prefix or postfix for members helps.
I prefer using `m_`as a prefix but a simple `_` as a postfix also works great.

### Ordering [-werror=reorder]

Member variables are being initialised in the order they are defined.
Using an alternative order in the constructor doesn`t change that and therefore makes the code hard to understand and debug.
Furthermore it introduces potential lifecycle problems since the destructor uses the reverse order. 
That means that the member that has been declared last will be destroyed first which might be relevant if a member `a` uses a member `b` but `a` is declared before `b`. 

### Sign Conversion [-werror=sign_conversion]

Comparing an unsigned integer like `size_t` and a signed integer may lead to unwanted behavior.
The compiler will implicitly convert the integer to `size_t` and converting a negative value like `-1` would result in unwanted behavior. 
I believe it would get very big but haven`t tested it so far. 