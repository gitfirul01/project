#!/usr/bin/python3

#from reportlab.pdfgen.canvas import Canvas
#from pdfrw import PdfReader
#from pdfrw.buildxobj import pagexobj
#from pdfrw.toreportlab import makerl

from fpdf import FPDF
from datetime import datetime
import os, csv

#WIDTH = 210
#HEIGHT = 297#   A4 SIZE

DATE = datetime.now().strftime("%d-%B-%Y, %H_%M_%S")
DIR = '/home/pi/LevellingPortable/'

DEVELOPER = 'Redesma Technologies'
PATH = '/home/pi/Desktop/DataReport/'
pdf_fileName = PATH + 'Levelling-Data-Report-' + DATE + '.pdf'
csv_fileName = 'data_logger'

def create_header(hari, self):
    self.image(DIR + 'assets/header.jpg', h=25)
    self.set_y(50)
    self.set_font('helvetica', 'B', 26)
    epw = self.w - 2*self.l_margin
    self.cell(epw, 0.0, 'Levelling Data Report', align='C')
    self.set_y(60)
    self.set_font('helvetica', 'I', 11)
    self.cell(epw, 0.0, DATE, align='C')

def create_footer(developer, self):
    # Page numbers in the footerself.WIDTH
    self.set_y(-15)
    self.set_font('times', 'I', 10)
    self.set_text_color(128)
    #self.cell(0, 10, 'Page ' + str(self.page_no()), 0, 0, 'C') #set page
    self.content = 'Produced by ' + developer
    self.cell(0,-15, self.content , 0, 0, 'C')

def create_body(self, operator, driver, nopol, volumeBefore, volumeAfter, deltaVolume):
    delta = deltaVolume
    epw = self.w -2 *self.l_margin +100
    self.set_font('helvetica', '', 16)
    
    def cell(x,y,word):
        self.set_xy(x,y)
        self.cell(epw, 0.0, word)
    
    #operator
    cell(40,90,'Operator');cell(95,90,f' : {operator}')
    #driver
    cell(40,100,'Driver');cell(95,100,f' : {driver}')
    #Nomor Polisi
    cell(40,110,'Nomor Polisi');cell(95,110,f' : {nopol}')
    #Volume Sebelum
    cell(40,120,'Volume Sebelum');cell(95,120,f' : {volumeBefore:.2f} L')

    #volume sesudah
    cell(40,130,'Volume Sesudah');cell(95,130,f' : {volumeAfter:.2f} L')

    cell(40,140,'Delta Volume');cell(95,140,f' : {delta:.2f} L')

def create_tembusan(self):
    epw = self.w - 2*self.l_margin + 100
    self.set_font('helvetica', '', 16)
    
    def cell(x,y,word):
        self.set_xy(x,y)
        self.cell(epw, 0.0, word)

    #direktur
    cell(30,210,'Tembusan:')
    cell(30,220,'-Manager Unit')
    cell(30,230,'-Supervisor')
    cell(30,240,'-Bag. Keuangan')

def create_analytics_report(operator, driver, nopol, volumeBefore, volumeAfter, deltaVolume):
    pdf = FPDF(format='A4', unit='mm')
    pdf.add_page()
    create_header(DATE, pdf)
    create_body(pdf, operator, driver, nopol, volumeBefore, volumeAfter, deltaVolume)
    create_tembusan(pdf)
    create_footer(DEVELOPER, pdf)
    pdf.output(pdf_fileName)
    #os.chmod(pdf_fileName, stat.S_IREAD) #make file executable //STILL NOT WORK
    #os.system(pdf_fileName)
    os.open( pdf_fileName, os.O_RDWR)
    #os.open(pdf_fileName, os.O_RDONLY)

    try:
        with open(PATH + csv_fileName + '.csv', newline='') as csvfile:
            csvfile.close()
        first = False
    except:
        first = True

    with open(PATH + csv_fileName + '.csv', 'a', newline='') as csvfile:
        timestamp = datetime.now().strftime("%d-%B-%Y %H:%M:%S")
        field_names = ['Timestamp', 'Operator', 'Driver', 'Nomor Polisi', 'Volume Sebelum', 'Volume Sesudah', 'Delta Volume']
        writer = csv.DictWriter(csvfile, fieldnames=field_names)

        if first == True:
            writer.writeheader()
        writer.writerow({'Timestamp': timestamp, 'Operator': operator, 'Driver': driver, 'Nomor Polisi': nopol, 'Volume Sebelum': volumeBefore, 'Volume Sesudah': volumeAfter, 'Delta Volume': deltaVolume})

        csvfile.close()
