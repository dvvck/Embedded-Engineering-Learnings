# Passkey

While working on a TCP/IP process interface client I stumbled across the `Passkey Idiom` which has been prominently published by [arne mertz](https://arne-mertz.de/2016/10/passkey-idiom/).

It`s purpose is to restrict the access of friends to member functions and enable the usage of smart pointers.

## Explanation

Friends are a clever construct in C++. 
They allow you to provide access to `private` or `protected` members.
The problem with that is, that you can not limit the access they get, it's all or nothing.
Furthermore if you make a constructor `private` or `protected` and provide access to friends you can not use `std::make_unique<>` because the constructor needs to be public for that.
This is where the passkey idiom shines.

The constructor is being declared in public space but needs a `TCPStreamKey`.
This key can only be constructed from friends of the `TCPStreamKey` class.
Which restricts the construction of the `TCPStream`class to the `TCPConnector`.

The full demo can be found [here](../../99_demos/passkey/main.cpp).
```cpp

class TCPStream
{
    class TCPStreamKey
    {
        friend class TCPConnector;

    private:
        TCPStreamKey() {};
        TCPStreamKey(TCPStreamKey const &) = default;
    };

public:
    //Public Constructor but only friends of the key can provide the needed parameter
    explicit TCPStream(int sd, struct sockaddr_in *address, TCPStreamKey);

private:
    int m_sd;
    string m_peerIp;
    int m_peerPort;
};
```



