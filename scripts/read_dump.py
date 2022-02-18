"""Read the dump files and parse them into plottable formats.
"""

def parse(filename):
    """parse a dump file into a table

    Args:
        filename (str): which file to open

    Returns:
        list: a list of the entries in the file.
            Entries are formatted as list while index 0 points to
            the argument, formatted as str, index 1 to the value (int)
    """
    with open(filename, 'r', encoding='UTF-8') as file_:
        data = file_.read()
        data = data.split('\n')
        table = []
        for line in data:
            entry = line.split(' ')
            if len(entry) == 2:
                entry[1] = int(entry[1])
                table.append(entry)
        return table

def format_to_dict(table):
    """group the entries of the table by arguments

    Args:
        table (list): list of [argument, value] pairs

    Returns:
        dict: the dictionary with argument as key and
            lists of values as value
    """
    dictionary = {}
    for entry in table:
        if entry[0] in dictionary:
            dictionary[entry[0]].append(entry[1])
        else:
            dictionary[entry[0]] = [entry[1]]
    return dictionary

def format_to_avg_dict(list_dict):
    """calculate the averages of each entry

    Args:
        list_dict (dict): the dict containing lists as every value, as from
            format_to_dict

    Returns:
        dict: keys are the same as in list_dict, values are averages (floats)
    """
    dictionary = {}
    for entry in list_dict:
        dictionary[entry] = sum(list_dict[entry]) / len(list_dict[entry])
    return dictionary

def read_dump(filename):
    """parse a dump file and return it in multiple formats

    Args:
        filename (str): path to dump file

    Returns:
        tuple: table(list), list_dict(dict with lists as values),
            avg_dict(dict with means as values)
    """
    table = parse(filename)
    list_dict = format_to_dict(table)
    avg_dict = format_to_avg_dict(list_dict)
    return table, list_dict, avg_dict
