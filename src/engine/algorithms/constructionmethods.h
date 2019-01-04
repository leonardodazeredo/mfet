#ifndef CONSTRUCTIONMETHODS_H
#define CONSTRUCTIONMETHODS_H

#include "solution.h"

class Instance;
class GenerationPhase;
class GenerationStrategy;

class ConstructionMethod : public QObject {
    Q_OBJECT
public:
    virtual Solution construct(float alpha = 0.0) = 0;

    virtual QString description() = 0;
    virtual QString shortDescription() const = 0;

    virtual ~ConstructionMethod(){}

    GenerationPhase *getParent() const;
    void setParent(GenerationPhase *value);

    int level() const;

    GenerationStrategy * getGenerationStrategy();

    void setGenerationStrategy(GenerationStrategy *value);

signals:
    void constructionStarted(ConstructionMethod* constructionMethod);
    void constructionReStarted(ConstructionMethod* constructionMethod);
    void constructionFinished(QString s, ConstructionMethod* constructionMethod);

protected slots:
    void logConstructionStarted(ConstructionMethod* constructionMethod);
    void logConstructionReStarted(ConstructionMethod* constructionMethod);
    void logConstructionFinished(QString s, ConstructionMethod* constructionMethod);

protected:
    Instance& instance;

    GenerationPhase* parent = nullptr;

    float alpha;

    ConstructionMethod(Instance& instance);

    GenerationStrategy* generationStrategy = nullptr;
};

class ConstructionBySubGroups : public ConstructionMethod{
public:
    ConstructionBySubGroups(Instance& instance) : ConstructionMethod(instance){}

    Solution construct(float alpha = 0.0);
    bool construct(Solution &solucao, float alpha);

    QString description();

    QString shortDescription() const;
};

class ConstructionByActivities : public ConstructionMethod{
public:
    ConstructionByActivities(Instance& instance) : ConstructionMethod(instance){}

    Solution construct(float alpha = 0.0);
    bool construct(Solution &solucao, float alpha);

    QString description();

    QString shortDescription() const;
};


#endif // CONSTRUCTIONMETHODS_H
