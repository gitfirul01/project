# For converting dict to csv lets create a sample list of dict. You can use your own dict
import xlsxwriter


sample_dict = {'key1': 1, 'key2': 2, 'key3': 3},


def csvSave(sample_dict):
    # The first step is to import python module for CSV and it is csv.
    import csv

    # Use DictWriter for writing the dicts into the list. Here we need to define the column name explicitly.
    col_name=["key1","key2","key3"]
    with open("expor.csv", 'w') as csvFile:
            wr = csv.writer(csvFile, dialect='excel')
            wr.writerow(col_name)
            for ele in sample_dict:
                wr.writerow(ele)


def pandasSave(sample_dict):
    import pandas as pd

    df = pd.DataFrame.from_dict(sample_dict) 
    df.to_csv (r'gen.csv', index = False, header=True)


# def xlutilsSave():
    # from xlrd import open_workbook
    # from xlutils.copy import copy

    # global sample_dict

    # rb = open_workbook(r"D:\MyFiles\redesma\data-acquisition\main\ref.xls")
    # wb = copy(rb)
    # s = wb.get_sheet(0)

    # s.write(1,0, 'key1')
    # s.write(1,1, 'a')

    # s.write(2,0, 'key2')
    # s.write(2,1, 'b')

    # s.write(3,0, 'key3')
    # s.write(3,1, 'c')

    # wb.save(r"D:\MyFiles\redesma\data-acquisition\main\data.xls")
# xlutilsSave()

def xlsxWriter():
    import xlsxwriter

    outWorkbook = xlsxwriter.Workbook(r"D:\MyFiles\redesma\data-acquisition\main\out.xlsx")
    outSheet = outWorkbook.add_worksheet()

    names = ['a', 'b', 'c']
    values = [1, 2, 3]

    outSheet.write(0,0, "Names")
    outSheet.write(0,1, "Scores")

    for item in range( len(names) ):
        outSheet.write(item+1, 0, names[item])
        outSheet.write(item+1, 1, values[item])
    
    # outSheet.write(0, 3, "Totals")
    # outSheet.write_formula(1, 3, "=SUM(B2:B4")

    outWorkbook.close()

xlsxWriter()