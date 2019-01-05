# m-FET - Timetabling software

This project is a product of the work required for the completion of the undergraduate course of Computer Science in the Universidade Federal Rural do Rio de Janeiro.

The undergraduate thesis associated to this project is included in this repository as well (available only in portuguese) and the following is its abstract:

> A common problem in all educational institutions in the world and recurrent to every school year is the definition of the timetables with encounters between teachers and students. Such problem, known as timetabling problem, is NP-Hard but still is mostly tackled manually by the schools, leading to waste of resources and frequently low quality solutions. This work present the problem in the context of university courses and develop a case study with the application of metaheuristics techniques, vastly used in the literature for solving the problem because of its complexity and the superior general performance that has been observed of the local search methodologies in relation to others. The performance of these methods is evaluated with real instances from the Departamento de Ciência da Computação of UFRRJ considering its specific needs such as the valorization of the teachers preferences for disciplines. The technique Simulated annealing and hybrid strategies of GRASP+VND and VNS+VND were experimented and, based on the results, other hybridizations were investigated. The results show that in the study case the methods implemented were able to easily surpass the quality of the solutions generated manually, according to the criteria used by the department, in a fraction of the time.

### Program description:

mFET is free timetabling software (licensed under the GNU Affero General Public License version 3 or later).
This program aims to automatically generate the timetable of a school, high-school or university.
It may be used for other timetabling purposes.

This software is based on the source code of FET version 5.31.6 (http://lalescu.ro/liviu/fet/) but significant changes were made to almost all parts of the original code including:

- complete replacement of the timetable generation techniques.
- removal of the space allocation feature.
- addition of automatic teacher assignment to activities based on their preferences for subjects.
- a lot of refactoring.


**Copyright (C) 2018 Leonardo Azeredo.**
