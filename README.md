<h1 align="center">C++: LFRU кэш</h1>
<h3 align="center">Лабораторная работа по курсу C++, ИТМО</h3>

## Least frequent recently used

Этот алгоритм применяется в областях, где требуется сбалансировать сохранение в кеше как часто используемых, так и недавно использованных элементов. Существуют разные вариации, здесь предложена одна из простых.

Кеш поделён на две области: привилегированная и непривилегированная. Привилегированная моделируется, как LRU список, а непривилегированная - как очередь FIFO.

Поиск производится сначала в привилегированной очереди (если элемент найден - он перемещается в её начало), затем - в непривилегированной (если элемент найден - он перемещается в привилегированную очередь).
Если элемент не найден, он добавляется в непривилегированную очередь.

Вытеснение из привилегированной очереди происходит так:
* удаляется самый редко используемый (последний) элемент из очереди
* он помещается в начало непривилегированной очереди (возможно, вызывая вытеснение там)

Для непривилегированной очереди новые элементы добавляются в начало, вытесненные удаляются с конца.

### Допущение в реализации
В реализации кеша допустимо предполагать, что все хранимые там объекты имеют в иерархии наследования предка, задаваемого шаблонным параметром KeyProvider,
который, в свою очередь, имеет оператор равенства с ключом (задаваемым шаблонным параметром Key).

## Модификация pool аллокатора с поддержкой нескольких типов объектов

Требуется расширить реализацию pool аллокатора возможностью размещать объекты нескольких разных типов (разного размера).
Возможны разные подходы к реализации такой идеи, но мы остановимся на варианте, напоминающем упрощённую модель [SLAB
аллокатора](https://en.wikipedia.org/wiki/Slab_allocation):
* пул состоит из набора блоков (slab)
* каждый блок используется для выделения памяти для объектов одного размера
* все блоки имеют одинаковый размер, указываемый при создании пула
* если в блоке кончается место, то попытка выделения памяти под объект соответствующего размера завершается неудачей

Пул должен создаваться с параметрами "размер блока" и "список размеров объектов". К расходу памяти предъявляются следующие требования:
* пусть пул создан с `N` блоков
* пусть размер блока равен `K`
* пусть размер элемента `i` блока равен `S(i)`
* тогда максимальный расход памяти на пул должен составлять `M <= N * (K + 8 * sizeof(void *)) + 10 * sizeof(void *) * sum(K / S(i) for i in range(N))`

Память под объект должна выделяться в блоке, выделенном для размера такого объекта.

Очевидно, что основным преимуществом такой схемы (в рамках задач выделения памяти под объекты разного размера) является отсутствие фрагментации. Основным
недостатком является негибкое выделение памяти - если разные размеры имеют разную частоту использования, то какие-то блоки могут простаивать с малой заполненностью.
Кроме того, если какой-то размер объектов не делит размер блока нацело, то часть памяти будет теряться впустую.
В более реалистичных реализациях блоки скорее всего будут выделяться динамически - когда один блок полностью заполнен, то запрос на выделение памяти под объект
соответствующего размера будет вызывать создание нового блока для обслуживания этого размера.

Подумайте, как можно оптимизировать служебные расходы памяти в таком аллокаторе и как ограничения стандарта языка мешают этой задаче. Можно ли обойти эти помехи,
если слегка пожертвовать универсальностью аллокатора?
