//
// Created by alex on 2/28/16.
//

#ifndef BDIF2016_PARAMETERS_H
#define BDIF2016_PARAMETERS_H


class Parameters {
public:
    bool output_called,output_diagnostic,output_evolve,output_exit,output_info,output_result;

    // constractor and initialization list
    Parameters():
            output_called(true),
            output_diagnostic(true),
            output_evolve(true),
            output_exit(true),
            output_info(true),
            output_result(true)
    {};

    // Methods

};


#endif //BDIF2016_PARAMETERS_H
