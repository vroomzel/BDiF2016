import pandas as pd
import numpy as np
import datetime,sys,getopt
import matplotlib.pyplot as plt
import scipy.stats as stats

# change matplotlib backend setting to play nicely with pycharm
plt.switch_backend('TkAgg') # TkAgg, Qt4Agg - seems to crash pycharm
plt.ion() # turn interactive mode on

# display pandas frames nicely
pd.set_option('max_columns',44)
pd.set_option('display.width',1000)
# pd.set_option('mode.chained_assignment',None)
# pd.options.display.float_format = '{:.8f}'.format

# file to explor
fname='data/data1000k.txt'
df=pd.DataFrame.from_csv(fname,header=None,infer_datetime_format=True)
df.index=pd.to_datetime(df.index,format='%Y%m%d:%H:%M:%S.%f')
df.columns=['px','vol']
df.index.name='dt'


df.px.describe()
df.vol.describe()
df.ix[(df.px.abs()<3000) | (df.px!=0)].px.plot()
df.ix[(df.px.abs()<3000) | (df.px!=0)].vol.plot()

# total outliers sprinkled in
df.ix[df.index.hour!=10]

# bad prices (different level of magnitude, negative and zero)
df.ix[(df.px.abs()>3000) | (df.px<=0)]

# without them
df.ix[(df.px.abs()<3000) & (df.px>0)].describe()

# quotes out of order
df['ts']=df.index
df.index=np.arange(0,len(df)) # to be able to slice the data better
y=pd.TimedeltaIndex(df.ts.diff()) # difference from prior quote
df['tdiff']=y.total_seconds() # translate into seconds cause Timedelta representation is intractable for comparison
x=df['tdiff'] # put in a separate time series
# play with it
x.ix[(x.abs()>1) & (x.abs()<2000)].hist()
x.ix[(x<-0.2) & (x>-2000)].hist()


# x=pd.to_timedelta(df.ts.diff())
# x=x.ix[x.abs()<pd.Timedelta(86400,units='s')]
# print x.ix[x>pd.Timedelta(1,unit='s')]