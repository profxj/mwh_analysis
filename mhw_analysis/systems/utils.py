""" Utility routines related to MHW Systems"""
import numpy as np
import pandas
from datetime import date
from collections import Counter
import warnings
from numba import njit, prange

from scipy.interpolate import interp1d
import healpy as hp

from oceanpy.sst import io as sst_io
from oceanpy.sst import utils as sst_utils

from IPython import embed

def dict_to_pandas(sys_dict, add_latlon=False, start_date=None):
    """

    Parameters
    ----------
    sys_dict : dict
        Contains all of the MHWS measurements
    add_latlon : bool, optional
        If True, load an NOAA OI and add lat, lon values to Table for convenience
    start_date : int, optional
        date.tooordinal value for the starting date of the cube
        If provided, add dates to the table

    Returns
    -------
    mhw_sys : pandas.DataFrame

    """
    mhw_sys = pandas.DataFrame()
    for key in sys_dict.keys():
        s = pandas.Series(sys_dict[key])
        mhw_sys[key] = s
    # Date?
    if start_date is not None:
        date_max = [date.fromordinal(start_date + int(zcen)) for zcen in mhw_sys['zcen'].values]
        mhw_sys['date'] = date_max

    # Lon/Lat
    if add_latlon:
        # Could hard code this
        lat_coord, lon_coord = sst_utils.noaa_oi_coords()
        f_lat = interp1d(np.arange(lat_coord.size), lat_coord)
        f_lon = interp1d(np.arange(lon_coord.size), lon_coord)
        # Evaluate
        warnings.warn("Fix the lat kludge!")
        mhw_sys['lat'] = f_lat(np.minimum(mhw_sys['xcen'].values, lat_coord.size-1))
        try:
            mhw_sys['lon'] = f_lon(np.minimum(mhw_sys['ycen'].values, lon_coord.size-1))
        except:
            embed(header='55 of utils')

    # Index
    mhw_sys = mhw_sys.set_index('Id')
    # Return
    return mhw_sys


@njit(parallel=True)
def max_area(mask, obj_id, areas):

    for kk, id in enumerate(obj_id):
        idx = np.where(mask == id)
        unique = np.unique(idx[2])
        counts = 0
        for jj in unique:
            counts = max(counts, np.sum(idx[2]==jj))
        areas[kk] = counts


def prep_labels(mask, parent, NVox, MinNVox=0, verbose=False):
    """

    Parameters
    ----------
    mask
    parent
    NVox
    MinNVox
    verbose

    Returns
    -------

    """
    # !..this is the number of individual connected components found in the cube:
    nlabels = np.max(mask)
    nobj=np.sum(parent[1:nlabels+1]==0)
    if verbose:
        print("NObj Extracted=",nobj)

    LabelToId = np.zeros(nlabels+1, dtype=np.int32) -1
    IdToLabel = np.zeros(nobj, dtype=np.int32)

    #!----- DETECTION (using NVox) -------------
    # !..build auxiliary arrays and count detections
    ndet=0
    for i in range(1,nlabels+1):
        if parent[i] == 0:
            this_label = i
            this_NVox = NVox[this_label] # 0-indexing
            if this_NVox > MinNVox:
                IdToLabel[ndet] = this_label
                LabelToId[this_label] = ndet
                ndet = ndet + 1  # ! update ndet
    if verbose:
        print('Nobj Detected =', ndet)

    # Return
    return IdToLabel, LabelToId, ndet


def mhw_sys_to_healpix(mhw_sys, nside):
    """
    Generate a healpix map of where the input
    MHW Systems are located on the globe

    Parameters
    ----------
    mhw_sys : pandas.DataFrame
    nside : int

    Returns
    -------
    healpix_array : hp.ma

    """

    # NOAA Coords
    lat, lon = sst_utils.noaa_oi_coords()
    f_lat = interp1d(np.arange(lat.size), lat)
    f_lon = interp1d(np.arange(lon.size), lon)

    lats = f_lat(np.minimum(mhw_sys['xcen'].values, lat.size-1))
    lons = f_lon(np.minimum(mhw_sys['ycen'].values, lon.size-1))

    # Healpix coords
    theta = (90 - lats) * np.pi / 180.
    phi = lons * np.pi / 180.
    idx_all = hp.pixelfunc.ang2pix(nside, theta, phi)

    # Count events
    npix_hp = hp.nside2npix(nside)
    all_events = np.ma.masked_array(np.zeros(npix_hp, dtype='int'))
    for idx in idx_all:
        all_events[idx] += 1

    # Mask

    # Return
    return hp.ma(all_events.astype(float))