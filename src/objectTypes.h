class CObject;

// The various types of object pointers and iterators
typedef std::tr1::shared_ptr<CObject>      objectPtr;
typedef std::tr1::weak_ptr<CObject>        objectPtrWeak;
typedef std::list<objectPtr>::iterator     objectIter;
typedef std::list<objectPtrWeak>::iterator objectIterWeak;