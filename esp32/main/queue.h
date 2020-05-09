struct QNode {
    float realPower;
    float apparentPower;
    float powerFactor;
    float Vrms;
    float Irms;
    struct QNode* next;
};


struct Queue {
    struct QNode *front, *rear;
};


struct QNode* newNode(const float realPower,
                      const float apparentPower,
                      const float powerFactor,
                      const float Vrms,
                      const float Irms) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->realPower = realPower;
    temp->apparentPower = apparentPower;
    temp->powerFactor = powerFactor;
    temp->Vrms = Vrms;
    temp->Irms = Irms;
    temp->next = NULL;
    return temp;
}


struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}


void enQueue(struct Queue* q,
              const float realPower,
              const float apparentPower,
              const float powerFactor,
              const float Vrms,
              const float Irms) {

    struct QNode* temp = newNode(realPower, apparentPower, powerFactor, Vrms, Irms);


    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }


    q->rear->next = temp;
    q->rear = temp;
}


float read_realPower(struct Queue* q) {
    return q->front->realPower;
}


float read_apparentPower(struct Queue* q) {
    return q->front->apparentPower;
}


float read_powerFactor(struct Queue* q) {
    return q->front->powerFactor;
}


float read_Vrms(struct Queue* q) {
    return q->front->Vrms;
}


float read_Irms(struct Queue* q) {
    return q->front->Irms;
}


void deQueue(struct Queue* q)
{

    if (q->front == NULL)
        return;

    struct QNode* temp = q->front;

    q->front = q->front->next;


    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}



