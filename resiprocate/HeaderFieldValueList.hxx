#ifndef HFVLIST_HXX
#define HFVLIST_HXX

#include <ostream>
#include <sip2/sipstack/HeaderFieldValue.hxx>

namespace Vocal2
{

class HeaderFieldValueList
{
   public:
      HeaderFieldValueList();

      HeaderFieldValueList(const HeaderFieldValueList& other);
      
      ~HeaderFieldValueList();
      
  // insert at begining
  void insert(HeaderFieldValue* header);

  // append to end
  void append(HeaderFieldValue* header);

  void deleteFirst();

      HeaderFieldValue* first;
  
  HeaderFieldValue* last;
};

}

std::ostream& operator<<(std::ostream& stream, 
			 Vocal2::HeaderFieldValueList& hList);

#endif
