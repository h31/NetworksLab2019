# Система новостей 

## Возможности клиента

* Запрос списка тем
* Запрос списка новостей по теме
* Запрос текста новости по теме и названию
* Добавление новости на сервер

##  <a name="a_packet_types">Типы пакетов</a>

  |Type number|  Type name                                    |
  |:---------:|:----------------------------------------------|
  | 01        | [Get list of topics](#a_get_list_of_topics)   |
  | 02        | [Get list of news](#a_get_list_of_news)       |
  | 03        | [Get news](#a_get_news)                       |
  | 04        | [Add topic](#a_add_topic)                     |
  | 05        | [Add news](#a_add_news)                       |
  | 06        | [List of topics](#a_list_of_topics)           |
  | 07        | [List of news](#a_list_of_news)               |
  | 08        | [News](#a_news)                               |
  | 09        | [Error](#a_error)                             |
  | 10        | [Acknowledgment](#a_acknowledgment)           |
  
  ### Посылаемые клиентом
  
  * <a name="a_get_list_of_topics">**Get list of topics**</a>
    
    ***Запрос списка новостных тем.***
    
    **Формат пакета:**
   
      | length | type |
      |:------:|:----:|
      |   4b   |  2b  |
      
      * Length - длина пакета 
      * [Type - тип пакета](#a_packet_types)
  
  * <a name="a_get_list_of_news">**Get list of news**</a>
  
    ***Запрос списка новостей (заголовков) по определённой теме.*** 
    
    **Формат пакета:**
    
    | length | type | topic | \0 |
    |:------:|:----:|:-----:|:--:|
    |   4b   |  2b  | string| 1b |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
  
  * <a name="a_get_news">**Get news**</a>
  
    ***Запрос текста новости по определённой теме.***
    
    **Формат пакета:**
    
    | length | type |  topic  | \0 |   header   | \0 |
    |:------:|:----:|:-------:|:--:|:----------:|:--:|
    |   4b   |  2b  | string  | 1b |   string   | 1b |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
    * Header - заголовок новости
    
  * <a name="a_add_topic">**Add topic**</a>
  
    ***Добавить новостную тему.***
    
    **Формат пакета:**
    
    | length | type | topic | \0 |
    |:------:|:----:|:-----:|:--:|
    |   4b   |  2b  |string | 1b |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
   
  * <a name="a_add_news">**Add news**</a>
  
    ***Добавить новость по определённой теме.***
    
    **Формат пакета:**
    
    | length | type | topic | \0 |   header   | \0 | news text |
    |:------:|:----:|:-----:|:--:|:----------:|:--:|:---------:|
    |   4b   |  2b  |string | 1b |   string   | 1b |    text   |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
    * Header - заголовок новости 
    * New text - текст новости
  
  ### Посылаемые сервером
  
  * <a name="a_list_of_topics">**List of topics**</a>
  
    ***Передача списка новостных тем.***
    
    **Формат пакета:**
    
    | length | type | topic | \0 |...|  topic  | \0 |
    |:------:|:----:|:-----:|:--:|:-:|:-------:|:--:|
    |   4b   |  2b  | string| 1b |...|  string | 1b |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
  
  * <a name="a_list_of_news">**List of news**</a>
  
    ***Передача списка заголовков новостей по данной теме.***
    
    **Формат пакета:**
    
    | length | type | topic | \0 |   header   | \0 |...|   header    | \0 |
    |:------:|:----:|:-----:|:--:|:----------:|:--:|:-:|:-----------:|:--:|
    |   4b   |  2b  |string | 1b |   string   | 1b |...|    string   | 1b |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
    * Header - заголовок новости
  
  * <a name="a_news">**News**</a>
  
     ***Передача текста новости.***
     
     **Формат пакета:**
     
    | length | type | topic | \0 |    header   | \0 | news text |
    |:------:|:----:|:-----:|:--:|:-----------:|:--:|:---------:|
    |   4b   |  2b  | string| 1b |    string   | 1b |    text   |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Topic - новостная тема
    * Header - заголовок новости
    * New text - текст новости
  
  * <a name="a_error">**Error**</a>
  
    ***Информирование об ошибке.***
    
    **Формат пакета:**
    
    | length | type | err type | err msg | \0 |
    |:------:|:----:|:--------:|:-------:|:--:|
    |   4b   |  2b  |    2b    |  string | 1b |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Err type - тип ошибки
    * Err msg - текст сообщения об ошибке 
    
    #### Типы сообщений об ошибках:
    
    |Type number|  Type name                                    |
    |:---------:|:----------------------------------------------|
    | 01        | Отсутствие новостной темы                     |
    | 02        | Отсутствие новости с данным заголовком        |
    | 03        | Добавление существующей новостой темы         |
    | 04        | Добавление новости с существующим заголовком  |
    
 
 
 
  * <a name="a_acknowledgment">**Acknowledgment**</a>
  
    ***Подтверждение успешного совершения действия.***
    
    **Формат пакета:**
    
    | length | type | ack type |
    |:------:|:----:|:--------:|
    |   4b   |  2b  |    2b    |
    
    * Length - длина пакета 
    * [Type - тип пакета](#a_packet_types)
    * Ack type - тип подтверждения
    
    #### Типы подтверждения:
    
    |Type number|  Type name                                    |
    |:---------:|:----------------------------------------------|
    | 01        | Новостная тема успешно добавлена              |
    | 02        | Новость успешно добавлена                     |
    
