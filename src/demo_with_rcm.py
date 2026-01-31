import numpy as np
from matplotlib import pyplot as plt
from matplotlib.ticker import MultipleLocator
from rcm import *

plt.rcParams.update(
    {
        "font.family": "Times New Roman",
        "font.size": 26,
        "mathtext.fontset": "cm",
    }
)

calculator = Calculator(thread=8)  # define a calculator with thread=8

# define a polar grid creator
grid = PolarGrid(rmin=0.01, rmax=30, rbinnum=40, phibinnum=16)
gridCoords = grid.coords()

# calculate the radial accelerations
accDicts = calculator.get_accs(filename="snapshot_000.hdf5", testPos=gridCoords)

# plot the figure
Rs, Phis = grid.rs_phis()  # get the grid data
leftMargin = 2
rightMargin = 2
lowerMargin = 2
upperMargin = 2
w = 8
h = w * 0.618
W = w + leftMargin + rightMargin
H = h + lowerMargin + upperMargin
# create the canvus
fig, ax = plt.subplots(nrows=1, ncols=1, figsize=(W, H))
ax.set_position([leftMargin / W, lowerMargin / H, w / W, h / H])
# basic frame
ax.set_xlabel(r"$R$ [kpc]")
ax.set_ylabel(r"$V_c$ [km/s]")
ax.set_xlim(0, 31)
ax.set_ylim(0, 250)

ax.xaxis.set_major_locator(MultipleLocator(5))
ax.xaxis.set_major_formatter("{x:.0f}")
ax.xaxis.set_minor_locator(MultipleLocator(1))
ax.yaxis.set_major_locator(MultipleLocator(50))
ax.yaxis.set_major_formatter("{x:.0f}")
ax.yaxis.set_minor_locator(MultipleLocator(10))

partTypes = [f"PartType{i}" for i in [1, 2]]
color = "black"
Rs, Phis = grid.rs_phis()
acc_comps = []
for partType in partTypes:
    accs = accDicts[partType]
    acc_comps.append(accs)
    rvs_local = np.sqrt(-Rs.flatten() * accs).reshape(Rs.shape)
    rvs_local = np.nanmean(rvs_local, axis=0)
    ax.plot(Rs[0], rvs_local, "x-", label=partType)

acc_comps = np.array(acc_comps)
acc_total = np.sum(acc_comps, axis=0)
rvs_total = np.sqrt(-Rs.flatten() * acc_total).reshape(Rs.shape)
rvs_total_mean = np.nanmean(rvs_total, axis=0)
ax.plot(Rs[0], rvs_total_mean, "x-", c="grey", label="Total")

ax.legend()
plt.show()
