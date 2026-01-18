## (DE) Steuerungssystem für das elektromagnetische Torschloss

Dieses Gerät auf Basis des Mikrocontrollers **ATtiny13a** dient der erfolgreichen Steuerung eines elektromagnetischen Schlosses in einem **Mehrfamilienhaus**. Es bietet eine zuverlässige Modusumschaltung sowie eine visuelle und akustische Statusanzeige.

### **Hardware-Merkmale**

* **Stromversorgung:** Das Gerät wird mit einer Spannung von ca. **\+22V** betrieben (Information aus der Historie).  
* **Mikrocontroller:** ATtiny13a (1,2 MHz).  
* **Schlosssteuerung:** Erfolgt über **ULN2003A**\-Treiber 1. Ein hoher Pegel an PB4 verriegelt das Schloss, ein niedriger Pegel öffnet es.  
* **Dämpfer:** Parallel zur Spule des Schlosses ist ein unpolarer Metallfilmkondensator (**0,5 – 1,0 µF, 400V**) als Dämpfer geschaltet (Information aus der Historie).  
* **Anzeige:** Ein aktiver Summer (PB3) und eine LED (PB2) arbeiten synchron.

### **Dokumentation**

* 📄 **Elektrischer Schaltplan** — detaillierte Beschreibung der Schaltung und der Anschlüsse (kalitka.pdf).  
* 📐 **Technische Zeichnung** — Abmessungen und konstruktive Merkmale (kalitka-draft.pdf).  
* 📝 **Lastenheft** — vollständige Algorithmen, Zeitintervalle und Programmlogik (requirements.md).

### **Kurzanleitung für den Benutzer**

Das System wird über eine einzige Taste gesteuert:

1. **Modus wechseln:** Halten Sie die Taste **3 Sekunden** lang gedrückt, um zwischen dem Modus „Immer offen“ (P1) und „Sicherheit“ (P2) zu wechseln.  
2. **Modus „Immer offen“ (P1):** Das Schloss ist dauerhaft stromlos (entriegelt). Das Gerät blinkt langsam und gibt kurze Signale ab.  
3. **Modus „Sicherheit“ (P2):** Das Schloss ist verriegelt. Bei einem **kurzen Tastendruck** unterbrechen die Treiber den Stromkreis des Schlosses für **4 Sekunden**, sodass die Tür geöffnet werden kann. Dies wird durch schnelles Blinken und einen Ton begleitet.

## (RU) Система управления электромагнитным замком калитки

Данное устройство на базе микроконтроллера **ATtiny13a** 1 успешно применяется для управления доступом во дворе **малоквартирного дома**. Система обеспечивает надежное переключение режимов работы и визуально-звуковую индикацию состояний.

### **Аппаратные особенности**

* **Питание:** На устройство подается напряжение около **\+22В** (информация из истории переписки).  
* **Микроконтроллер:** ATtiny13a (1.2 МГц).  
* **Управление замком:** Осуществляется через ключи **ULN2003A**. Подача высокого уровня на PB4 запирает замок, снятие напряжения (низкий уровень) — открывает.  
* **Демпфер:** Параллельно катушке замка включен неполярный металлопленочный конденсатор (**0.5 – 1.0 мкФ, 400В**) для гашения обратных токов (информация из истории переписки).  
* **Индикация:** Активный зуммер (PB3) и светодиод (PB2) работают синхронно.

### **Документация проекта**

* 📄 **Схема электрическая принципиальная** — подробное описание цепей и подключения (kalitka.pdf).  
* 📐 **Чертеж устройства** — габариты и конструктивное исполнение (kalitka-draft.pdf).  
* 📝 **Техническое задание** — подробное описание алгоритмов, таймингов и логики работы (requirements.md).

### **Краткое руководство пользователя**

Управление системой осуществляется одной кнопкой 1:

1. **Смена режима:** Удерживайте кнопку **3 секунды**, чтобы переключиться между «Постоянно открыто» (Р1) и «Охрана» (Р2).  
2. **Режим «Постоянно открыто» (P1):** Замок постоянно обесточен (разблокирован). Устройство медленно мигает и подает короткие звуковые сигналы.  
3. **Режим «Охрана» (P2):** Замок заперт (находится под напряжением). При **коротком нажатии** на кнопку ключи размыкают цепь питания электромагнитного замка на **4 секунды**, позволяя открыть дверь. Это время сопровождается быстрым миганием светодиода и звуковым сигналом.

