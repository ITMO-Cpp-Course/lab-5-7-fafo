#pragma once 

#include <string> 


class Document
{
  public:
    Document() = default;

    Document(int id, std::string name, std::string text)
        : id_(id) 
          ,
          name_(std::move(name)) 
          ,
          text_(std::move(text)) 
    {
    }


    int id() const
    {
        return id_;
    }


    const std::string& name() const
    {
        return name_;
    }

   
    const std::string& text() const
    {
        return text_;
    }

  private:
    int id_ = 0;       
    std::string name_; 
    std::string text_; 
};