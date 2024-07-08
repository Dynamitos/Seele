import pandas as pd
import matplotlib.pyplot as plt

#areas = pd.read_csv('build/s.csv')
#
#plot = areas.hist(column='area', bins=100)
#plt.yscale('log')
#plt.show()

df = pd.read_csv('build/stats.csv')
noculldf = pd.read_csv('build/statsNOCULL.csv')

fig, ax = plt.subplots()

scaled_reltime = df['RelTime'] / 10**9

scaled_FrameTime = df['FrameTime'].rolling(window=100).mean() / 10**9
scaled_CACHED = df['CACHED'].rolling(window=100).mean() / 10**9
scaled_MIPGEN = df['MIPGEN'].rolling(window=100).mean() / 10**9
scaled_DEPTHCULL = df['DEPTHCULL'].rolling(window=100).mean() / 10**9
scaled_VISIBILITY = df['VISIBILITY'].rolling(window=100).mean() / 10**9
scaled_LIGHTCULL = df['LIGHTCULL'].rolling(window=100).mean() / 10**9
scaled_BASE = df['BASE'].rolling(window=100).mean() / 10**9

ax.plot(scaled_reltime, scaled_FrameTime, label='Frame Time')
ax.plot(scaled_reltime, scaled_CACHED, label='CACHED')
ax.plot(scaled_reltime, scaled_MIPGEN, label='MIPGEN')
ax.plot(scaled_reltime, scaled_DEPTHCULL, label='DEPTHCULL')
ax.plot(scaled_reltime, scaled_VISIBILITY, label='VISIBILITY')
ax.plot(scaled_reltime, scaled_LIGHTCULL, label='LIGHTCULL')
ax.plot(scaled_reltime, scaled_BASE, label='BASE')



ax.set_xlabel('Application Time (ms)')
ax.set_ylabel('Render Times (ms)')

ax.legend()

fig.savefig('allcull.png')

nocullfig, nocullax = plt.subplots()

nocullscaled_reltime = noculldf['RelTime'] / 10**9

nocullscaled_FrameTime = noculldf['FrameTime'].rolling(window=100).mean() / 10**9
nocullscaled_CACHED = noculldf['CACHED'].rolling(window=100).mean() / 10**9
nocullscaled_MIPGEN = noculldf['MIPGEN'].rolling(window=100).mean() / 10**9
nocullscaled_DEPTHCULL = noculldf['DEPTHCULL'].rolling(window=100).mean() / 10**9
nocullscaled_VISIBILITY = noculldf['VISIBILITY'].rolling(window=100).mean() / 10**9
nocullscaled_LIGHTCULL = noculldf['LIGHTCULL'].rolling(window=100).mean() / 10**9
nocullscaled_BASE = noculldf['BASE'].rolling(window=100).mean() / 10**9

nocullax.plot(nocullscaled_reltime, nocullscaled_FrameTime, label='Frame Time')
nocullax.plot(nocullscaled_reltime, nocullscaled_CACHED, label='CACHED')
nocullax.plot(nocullscaled_reltime, nocullscaled_MIPGEN, label='MIPGEN')
nocullax.plot(nocullscaled_reltime, nocullscaled_DEPTHCULL, label='DEPTHCULL')
nocullax.plot(nocullscaled_reltime, nocullscaled_VISIBILITY, label='VISIBILITY')
nocullax.plot(nocullscaled_reltime, nocullscaled_LIGHTCULL, label='LIGHTCULL')
nocullax.plot(nocullscaled_reltime, nocullscaled_BASE, label='BASE')

nocullax.set_xlabel('Application Time (ms)')
nocullax.set_ylabel('Render Times (ms)')

nocullax.legend()

nocullfig.savefig('allnocull.png')

combfig, combax = plt.subplots()

combax.plot(scaled_reltime, scaled_FrameTime, label='Culling Frametime')
combax.plot(nocullscaled_reltime, nocullscaled_FrameTime, label='No Culling Frametime')

combax.set_xlabel('Application Time (ms)')
combax.set_ylabel('Render Times (ms)')

combax.legend()

combfig.savefig('combined.png')


