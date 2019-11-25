# Электронный магазин

## Возможности сервера 

* Отключение лиента
* Продажа товара
* Добавление нового товара
* Предоставление клиенту списка товаров

## Типы пакетов

  |Type number|  Type name                                    |
  |:---------:|:---------------------------------------------:|
  | 01        | [error](#a_error)                             |
  | 02        | [list of products](#a_list_of_products)       |
  | 03        | [add product](#a_add_product)                 |
  | 04        | [buy product](#a_buy_product)                 |
  | 05        | [acknowledgment](#a_acknowledgment)           |
  | 06        |[get list of products](#a_get_list_of_products)|
  
* <a name="a_error">**Error**</a>

  ***Посылается сервером при отсутствии запрашиваемого товара.***
  
  **Формат пакета:**
  
  |length| type | error message| \0 |
  |:----:|:----:|:------------:|:--:|
  |  2b  |  2b  |    string    | 1b |

  * Length - длина пакета
  * Type - тип пакета ([error](#a_error), [list of products](#a_list_of_products), [add product](#a_add_product), [buy product](#a_buy_product), [acknowledgment](#a_acknowledgment), [get list of products](#a_get_list_of_products))
  * Error message - текст сообщения об ошибке
  
* <a name="a_list_of_products">**List of products**</a>

  ***Посылается сервером в ответ на запрос списка товаров.***
  
  **Формат пакета:**  

  | length | type | cost | count | name | \0 |...| cost | count | name | \0 |
  |:------:|:----:|:----:|:-----:|:----:|:--:|:-:|:----:|:-----:|:----:|:--:|
  |   2b   |  2b  |  2b  |   2b  |string| 1b |...|  2b  |   2b  |string| 1b |

  * Length - длина пакета
  * Type - тип пакета ([error](#a_error), [list of products](#a_list_of_products), [add product](#a_add_product), [buy product](#a_buy_product), [acknowledgment](#a_acknowledgment), [get list of products](#a_get_list_of_products))
  * Cost - цена данного товара
  * Count - количество данного товара 
  * Name - название данного товара
  
* <a name="a_add_product">**Add product**</a>

  ***Посылается клиентом для добавления товара в список.***  
  ***Если товар с таким названием уже имеется, то его количество увеличивается на количество добавляемого товара.***  
  ***В ответ на данный запрос клиенту посылается*** ["acknowledgment"](#a_acknowledgment) ***пакет с указанием количества добавленного товара.***
  
  **Формат пакета:**
  
  | length | type |count|price| name |\0 | 
  |:------:|:----:|:---:|:---:|:----:|:-:|
  |   2b   |  2b  |  2b |  2b |string| 1b|

  * Length - длина пакета
  * Type - тип пакета ([error](#a_error), [list of products](#a_list_of_products), [add product](#a_add_product), [buy product](#a_buy_product), [acknowledgment](#a_acknowledgment), [get list of products](#a_get_list_of_products))
  * Count - количество данного товара
  * Price - цена данного товара
  * Name - название данного товара
  
* <a name="a_buy_product">**Buy product**</a>
  
  ***Посылается клиентом для покупки товара.***  
  ***Если имеющееся количество товара меньше запрашиваемого, то продаётся всё имеющееся количество товара.***  
  ***В ответ на данный запрос клиенту посылается*** ["acknowledgment"](#a_acknowledgment) ***пакет с указанием количества купленного товара.***
  
  **Формат пакета:**
  
  | length | type | count | name | \0 | 
  |:------:|:----:|:-----:|:----:|:--:|
  |   2b   |  2b  |   2b  |string| 1b |

  * Length - длина пакета
  * Type - тип пакета ([error](#a_error), [list of products](#a_list_of_products), [add product](#a_add_product), [buy product](#a_buy_product), [acknowledgment](#a_acknowledgment), [get list of products](#a_get_list_of_products))
  * Count - количество данного товара
  * Name - название данного товара
  
* <a name="a_acknowledgment">**Acknowledgment**</a>
  
  ***Посылается сервером в ответ на запрос покупки/добавления товара.***
  
  **Формат пакета:**
  
  | length | type | ack. type | count |
  |:------:|:----:|:---------:|:-----:|
  |   2b   |  2b  |     2b    |   2b  |
  
  * Length - длина пакета
  * Type - тип пакета ([error](#a_error), [list of products](#a_list_of_products), [add product](#a_add_product), [buy product](#a_buy_product), [acknowledgment](#a_acknowledgment), [get list of products](#a_get_list_of_products))
  * Acknowledgment type - тип подтверждения (товар куплен, товар добавлен)
  * Count - количество данного товара
  
* <a name="a_get_list_of_products">**Get list of products**</a>
  
  ***Посылается клиентом для запроса списка товаров.***  
  ***В ответ на данный запрос клиенту посылается пакет типа*** ["List of products"](#a_list_of_products) ***.***
  
  **Формат пакета:**
  
  | length | type |
  |:------:|:----:|
  |   2b   |  2b  |
  
  * Length - длина пакета
  * Type - тип пакета ([error](#a_error), [list of products](#a_list_of_products), [add product](#a_add_product), [buy product](#a_buy_product), [acknowledgment](#a_acknowledgment), [get list of products](#a_get_list_of_products))
  
