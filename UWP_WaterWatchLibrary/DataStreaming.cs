/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.ComponentModel;

// using EDMS_App.Data;
using Microsoft.Toolkit.Collections;
using Windows.Foundation;
using Windows.UI.Xaml.Data;
using UWP_WaterWatchLibrary;

namespace UWP_WaterWatchLibrary
{
    //
    // Summary:
    //     This interface represents a data source whose items can be loaded incrementally.
    //
    // Type parameters:
    //   TSource:
    //     Type of collection element.
    public interface cweeIIncrementalSource<TSource>
    {
        //
        // Summary:
        //     This method is invoked every time the view need to show more items. Retrieves
        //     items based on pageIndex and pageSize arguments.
        //
        // Parameters:
        //   pageIndex:
        //     The zero-based index of the page that corresponds to the items to retrieve.
        //
        //   pageSize:
        //     The number of TSource items to retrieve for the specified pageIndex.
        //
        //   cancellationToken:
        //     Used to propagate notification that operation should be canceled.
        //
        // Returns:
        //     Returns a collection of TSource.
        Task<IEnumerable<TSource>> GetPagedItemsAsync(int pageIndex, int pageSize, object Tag = null);
    }
    public interface cweeDeferredIIncrementalSource<TSource>
    {
        //
        // Summary:
        //     This method is invoked every time the view need to show more items. Retrieves
        //     items based on pageIndex and pageSize arguments.
        //
        // Parameters:
        //   pageIndex:
        //     The zero-based index of the page that corresponds to the items to retrieve.
        //
        //   pageSize:
        //     The number of TSource items to retrieve for the specified pageIndex.
        //
        //   cancellationToken:
        //     Used to propagate notification that operation should be canceled.
        //
        // Returns:
        //     Returns a collection of TSource.
        cweeTask<IEnumerable<TSource>> GetPagedItemsAsync(int pageIndex, int pageSize, object Tag = null);
    }

    /// <summary>
    /// This class represents an <see cref="ObservableCollection{IType}"/> whose items can be loaded incrementally.
    /// </summary>
    /// <typeparam name="TSource">
    /// The data source that must be loaded incrementally.
    /// </typeparam>
    /// <typeparam name="IType">
    /// The type of collection items.
    /// </typeparam>
    /// <seealso cref="IIncrementalSource{TSource}"/>
    /// <seealso cref="ISupportIncrementalLoading"/>
    public class cweeIncrementalLoader<TSource, IType> : ObservableCollection<IType>, ISupportIncrementalLoading where TSource : cweeIIncrementalSource<IType>
    {
        private readonly AtomicInt _mutex = new AtomicInt();

        public object Tag { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called when a retrieval operation begins.
        /// </summary>
        public Action OnStartLoading { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called when a retrieval operation ends.
        /// </summary>
        public Action OnEndLoading { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called if an error occurs during data retrieval. The actual <see cref="Exception"/> is passed as an argument.
        /// </summary>
        public Action<Exception> OnError { get; set; }

        /// <summary>
        /// Gets a value indicating the source of incremental loading.
        /// </summary>
        protected TSource Source { get; }

        /// <summary>
        /// Gets a value indicating how many items that must be retrieved for each incremental call.
        /// </summary>
        protected int ItemsPerPage { get; }

        /// <summary>
        /// Gets or sets a value indicating The zero-based index of the current items page.
        /// </summary>
        protected int CurrentPageIndex { get; set; }

        private bool _isLoading;
        private bool _hasMoreItems;
        private CancellationToken _cancellationToken;
        private bool _refreshOnLoad;

        /// <summary>
        /// Gets a value indicating whether new items are being loaded.
        /// </summary>
        public bool IsLoading
        {
            get
            {
                return _isLoading;
            }

            private set
            {
                if (value != _isLoading)
                {
                    _isLoading = value;
                    OnPropertyChanged(new PropertyChangedEventArgs(nameof(IsLoading)));

                    if (_isLoading)
                    {
                        OnStartLoading?.Invoke();
                    }
                    else
                    {
                        OnEndLoading?.Invoke();
                    }
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the collection contains more items to retrieve.
        /// </summary>
        public bool HasMoreItems
        {
            get
            {
                if (_cancellationToken.IsCancellationRequested)
                {
                    return false;
                }

                return _hasMoreItems;
            }

            private set
            {
                if (value != _hasMoreItems)
                {
                    _hasMoreItems = value;
                    OnPropertyChanged(new PropertyChangedEventArgs(nameof(HasMoreItems)));
                }
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cweeIncrementalLoader{TSource, IType}"/> class optionally specifying how many items to load for each data page.
        /// </summary>
        /// <param name="itemsPerPage">
        /// The number of items to retrieve for each call. Default is 20.
        /// </param>
        /// <param name="onStartLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation begins.
        /// </param>
        /// <param name="onEndLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation ends.
        /// </param>
        /// <param name="onError">
        /// An <see cref="Action"/> that is called if an error occurs during data retrieval.
        /// </param>
        /// <seealso cref="IIncrementalSource{TSource}"/>
        public cweeIncrementalLoader(int itemsPerPage = 20, Action onStartLoading = null, Action onEndLoading = null, Action<Exception> onError = null)
            : this(Activator.CreateInstance<TSource>(), itemsPerPage, onStartLoading, onEndLoading, onError)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cweeIncrementalLoader{TSource, IType}"/> class using the specified <see cref="IIncrementalSource{TSource}"/> implementation and, optionally, how many items to load for each data page.
        /// </summary>
        /// <param name="source">
        /// An implementation of the <see cref="IIncrementalSource{TSource}"/> interface that contains the logic to actually load data incrementally.
        /// </param>
        /// <param name="itemsPerPage">
        /// The number of items to retrieve for each call. Default is 20.
        /// </param>
        /// <param name="onStartLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation begins.
        /// </param>
        /// <param name="onEndLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation ends.
        /// </param>
        /// <param name="onError">
        /// An <see cref="Action"/> that is called if an error occurs during data retrieval.
        /// </param>
        /// <seealso cref="IIncrementalSource{TSource}"/>
        public cweeIncrementalLoader(TSource source, int itemsPerPage = 20, Action onStartLoading = null, Action onEndLoading = null, Action<Exception> onError = null)
        {
            if (source == null)
            {
                throw new ArgumentNullException(nameof(source));
            }

            Source = source;

            OnStartLoading = onStartLoading;
            OnEndLoading = onEndLoading;
            OnError = onError;

            ItemsPerPage = itemsPerPage;
            _hasMoreItems = true;
        }

        /// <summary>
        /// Initializes incremental loading from the view.
        /// </summary>
        /// <param name="count">
        /// The number of items to load.
        /// </param>
        /// <returns>
        /// An object of the <see cref="LoadMoreItemsAsync(uint)"/> that specifies how many items have been actually retrieved.
        /// </returns>
        public IAsyncOperation<LoadMoreItemsResult> LoadMoreItemsAsync(uint count) => LoadMoreItemsAsync(count, Tag).AsAsyncOperation();

        /// <summary>
        /// Clears the collection and triggers/forces a reload of the first page
        /// </summary>
        /// <returns>This method does not return a result</returns>
        public Task RefreshAsync()
        {
            if (IsLoading)
            {
                _refreshOnLoad = true;
            }
            else
            {
                var previousCount = Count;
                Clear();
                CurrentPageIndex = 0;
                HasMoreItems = true;

                if (previousCount == 0)
                {
                    // When the list was empty before clearing, the automatic reload isn't fired, so force a reload.
                    return LoadMoreItemsAsync(0, Tag);
                }
            }

            return Task.CompletedTask;
        }

        /// <summary>
        /// Actually performs the incremental loading.
        /// </summary>
        /// <param name="cancellationToken">
        /// Used to propagate notification that operation should be canceled.
        /// </param>
        /// <returns>
        /// Returns a collection of <typeparamref name="IType"/>.
        /// </returns>
        protected virtual async Task<IEnumerable<IType>> LoadDataAsync(object _tag)
        {
            var result = await Source.GetPagedItemsAsync(CurrentPageIndex, ItemsPerPage, _tag)
                .ContinueWith(
                    t =>
                    {
                        if (t.IsFaulted)
                        {
                            throw t.Exception;
                        }

                        if (t.IsCompletedSuccessfully)
                        {
                            CurrentPageIndex += 1;
                        }
                        return t.Result;
                    });

            return result;
        }

        private async Task<LoadMoreItemsResult> LoadMoreItemsAsync(uint count, object _tag)
        {
            uint resultCount = 0;

            // TODO (2021.05.05): Make use common AsyncMutex class.
            // AsyncMutex is located at Microsoft.Toolkit.Uwp.UI.Media/Extensions/System.Threading.Tasks/AsyncMutex.cs at the time of this note.

            while (!_mutex.TryIncrementTo(1))
            {
                // EdmsCalls.Think();
            }

            try
            {
                if (!_cancellationToken.IsCancellationRequested)
                {
                    IEnumerable<IType> data = null;
                    try
                    {
                        IsLoading = true;
                        data = await LoadDataAsync(_tag);
                    }
                    catch (OperationCanceledException)
                    {
                        // The operation has been canceled using the Cancellation Token.
                    }
                    catch (Exception ex) when (OnError != null)
                    {
                        OnError.Invoke(ex);
                    }

                    if (data != null && data.Any() && !_cancellationToken.IsCancellationRequested)
                    {
                        resultCount = (uint)data.Count();

                        foreach (var item in data)
                        {
                            Add(item);
                        }
                    }
                    else
                    {
                        HasMoreItems = false;
                    }
                }
            }
            finally
            {
                IsLoading = false;

                if (_refreshOnLoad)
                {
                    _refreshOnLoad = false;
                    await RefreshAsync();
                }

                _mutex.Decrement();
            }

            return new LoadMoreItemsResult { Count = resultCount };
        }
    }

    /// <summary>
    /// This class represents an <see cref="ObservableCollection{IType}"/> whose items can be loaded incrementally.
    /// </summary>
    /// <typeparam name="TSource">
    /// The data source that must be loaded incrementally.
    /// </typeparam>
    /// <typeparam name="IType">
    /// The type of collection items.
    /// </typeparam>
    /// <seealso cref="IIncrementalSource{TSource}"/>
    /// <seealso cref="ISupportIncrementalLoading"/>
    public class cweeDeferredIncrementalLoader<TSource, IType> : ObservableCollection<IType>, ISupportIncrementalLoading where TSource : cweeDeferredIIncrementalSource<IType>
    {
        private readonly AtomicInt _mutex = new AtomicInt();

        public object Tag { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called when a retrieval operation begins.
        /// </summary>
        public Action OnStartLoading { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called when a retrieval operation ends.
        /// </summary>
        public Action OnEndLoading { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called when a retrieval operation ends.
        /// </summary>
        public Action<IEnumerable<IType>> OnFinishedAllLoading { get; set; }

        /// <summary>
        /// Gets or sets an <see cref="Action"/> that is called if an error occurs during data retrieval. The actual <see cref="Exception"/> is passed as an argument.
        /// </summary>
        public Action<Exception> OnError { get; set; }

        /// <summary>
        /// Should the streaming wait for the user to scroll the dataset, or should it continue streaming regardless? 
        /// </summary>
        public bool WaitForListScroll { get; set; }

        /// <summary>
        /// Gets a value indicating the source of incremental loading.
        /// </summary>
        protected TSource Source { get; }

        /// <summary>
        /// Gets a value indicating how many items that must be retrieved for each incremental call.
        /// </summary>
        protected int ItemsPerPage { get; }

        /// <summary>
        /// Gets or sets a value indicating The zero-based index of the current items page.
        /// </summary>
        protected int CurrentPageIndex { get; set; }

        private bool _isLoading;
        private bool _hasMoreItems;
        private CancellationToken _cancellationToken;
        private bool _refreshOnLoad;

        /// <summary>
        /// Gets a value indicating whether new items are being loaded.
        /// </summary>
        public bool IsLoading
        {
            get
            {
                return _isLoading;
            }

            private set
            {
                if (value != _isLoading)
                {
                    _isLoading = value;
                    EdmsTasks.InsertJob(() =>
                    {
                        OnPropertyChanged(new PropertyChangedEventArgs(nameof(IsLoading)));

                        if (_isLoading)
                        {
                            OnStartLoading?.Invoke();
                        }
                        else
                        {
                            OnEndLoading?.Invoke();
                        }
                    }, EdmsTasks.Priority.Low, true);
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the collection contains more items to retrieve.
        /// </summary>
        public bool HasMoreItems
        {
            get
            {
                if (_cancellationToken.IsCancellationRequested)
                {
                    return false;
                }

                return _hasMoreItems;
            }

            private set
            {
                if (value != _hasMoreItems)
                {
                    _hasMoreItems = value;
                    EdmsTasks.InsertJob(() =>
                    {
                        OnPropertyChanged(new PropertyChangedEventArgs(nameof(HasMoreItems)));
                    }, EdmsTasks.Priority.Low, true);
                }
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cweeIncrementalLoader{TSource, IType}"/> class optionally specifying how many items to load for each data page.
        /// </summary>
        /// <param name="itemsPerPage">
        /// The number of items to retrieve for each call. Default is 20.
        /// </param>
        /// <param name="onStartLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation begins.
        /// </param>
        /// <param name="onEndLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation ends.
        /// </param>
        /// <param name="onError">
        /// An <see cref="Action"/> that is called if an error occurs during data retrieval.
        /// </param>
        /// <seealso cref="IIncrementalSource{TSource}"/>
        public cweeDeferredIncrementalLoader(int itemsPerPage = 20, Action onStartLoading = null, Action onEndLoading = null, Action<Exception> onError = null, bool waitForListScroll = true, Action<IEnumerable<IType>> onFinishedAllLoading = null)
            : this(Activator.CreateInstance<TSource>(), itemsPerPage, onStartLoading, onEndLoading, onError, waitForListScroll, onFinishedAllLoading)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cweeIncrementalLoader{TSource, IType}"/> class using the specified <see cref="IIncrementalSource{TSource}"/> implementation and, optionally, how many items to load for each data page.
        /// </summary>
        /// <param name="source">
        /// An implementation of the <see cref="IIncrementalSource{TSource}"/> interface that contains the logic to actually load data incrementally.
        /// </param>
        /// <param name="itemsPerPage">
        /// The number of items to retrieve for each call. Default is 20.
        /// </param>
        /// <param name="onStartLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation begins.
        /// </param>
        /// <param name="onEndLoading">
        /// An <see cref="Action"/> that is called when a retrieval operation ends.
        /// </param>
        /// <param name="onError">
        /// An <see cref="Action"/> that is called if an error occurs during data retrieval.
        /// </param>
        /// <seealso cref="IIncrementalSource{TSource}"/>
        public cweeDeferredIncrementalLoader(TSource source, int itemsPerPage = 20, Action onStartLoading = null, Action onEndLoading = null, Action<Exception> onError = null, bool waitForListScroll = true, Action<IEnumerable<IType>> onFinishedAllLoading = null)
        {
            if (source == null)
            {
                throw new ArgumentNullException(nameof(source));
            }

            Source = source;

            OnStartLoading = onStartLoading;
            OnEndLoading = onEndLoading;
            OnFinishedAllLoading = onFinishedAllLoading;
            OnError = onError;
            WaitForListScroll = waitForListScroll;

            ItemsPerPage = itemsPerPage;
            _hasMoreItems = true;
        }

        /// <summary>
        /// Clears the collection and triggers/forces a reload of the first page
        /// </summary>
        /// <returns>This method does not return a result</returns>
        public cweeTask<LoadMoreItemsResult> RefreshAsync()
        {
            if (IsLoading)
            {
                _refreshOnLoad = true;
            }
            else
            {
                var previousCount = Count;
                return EdmsTasks.InsertJob(() =>
                {
                    Clear();
                }, true).ContinueWith(()=> {
                    CurrentPageIndex = 0;
                    HasMoreItems = true;

                    if (previousCount == 0)
                    {
                        // When the list was empty before clearing, the automatic reload isn't fired, so force a reload.
                        return LoadMoreItemsAsync(0, Tag);
                    }
                    else
                    {
                        return null;
                    }
                }, false);
            }
            return null;
        }

        private cweeTask<LoadMoreItemsResult> LoadMoreItemsAsync(uint count, object _tag)
        {
            // enter mutex
            while (!_mutex.TryIncrementTo(1))
            {
                // EdmsCalls.Think();
            }

            // do work
            {
                // Task B
                IsLoading = true;
                var B = Source.GetPagedItemsAsync(CurrentPageIndex, ItemsPerPage, _tag);
                var C = B.ContinueWith(() => {
                    CurrentPageIndex += 1;
                    int n = 0;
                    IEnumerable<IType> data = (IEnumerable<IType>)B.Result;
                    if (data != null && data.Any() && !_cancellationToken.IsCancellationRequested)
                    {
                        return EdmsTasks.InsertJob(() => {
                            foreach (var item in data) Add(item);
                        }, true).ContinueWith(() => {
                            n = data.Count();

                            IsLoading = false;
                            if (_refreshOnLoad)
                            {
                                _refreshOnLoad = false;
                                _mutex.Decrement();
                                return RefreshAsync();
                            }
                            else
                            {
                                // exit mutex                            
                                _mutex.Decrement();
                                return new LoadMoreItemsResult { Count = (uint)n };
                            }
                        }, false);  
                    }
                    else
                    {
                        HasMoreItems = false;

                        IsLoading = false;
                        if (_refreshOnLoad)
                        {
                            _refreshOnLoad = false;
                            _mutex.Decrement();
                            return RefreshAsync();
                        }
                        else
                        {
                            // exit mutex                            
                            _mutex.Decrement();
                            return new LoadMoreItemsResult { Count = (uint)n };
                        }
                    }
                }, false);

                if (!HasMoreItems)
                {
                    if (OnFinishedAllLoading != null)
                    {
                        OnFinishedAllLoading?.Invoke(this.ToList());
                    }
                }

                if (WaitForListScroll || !HasMoreItems)
                {
                    return C;
                }
                else
                {
                    C.ContinueWith(() => { LoadMoreItemsAsync(count, _tag); }, false);
                    return C;
                }
            }
        }

        IAsyncOperation<LoadMoreItemsResult> ISupportIncrementalLoading.LoadMoreItemsAsync(uint count)
        {
            return LoadMoreItemsAsync(count, Tag).AsAsyncOperation<LoadMoreItemsResult>();
        }
    } 
}
